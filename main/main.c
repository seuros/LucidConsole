/*
 * LucidConsole - WiFi-to-UART Bridge
 * Modular architecture with optional OLED display support
 */

#include "lucid_config.h"              // Feature configuration
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <string.h>

// Core components (always enabled)
#include "hardware/gpio_init.h"

// Optional display components (conditional compilation)
#if CONFIG_ENABLE_OLED_DISPLAY
#include "bus/i2c_hw_bus.h"
#include "display/oled_framebuffer.h"
#endif

// WiFi management
#include "wifi/wifi_manager.h"
#include "web/web_server.h"

// UART bridge system
#include "uart/uart_bridge.h"

static const char* TAG = "LUCIDUART";

#if CONFIG_ENABLE_OLED_DISPLAY
/**
 * @brief Show boot sequence with diagnostic test pattern (Display Mode)
 */
static void show_boot_sequence(void) {
    esp_err_t ret = oled_framebuffer_render_test_pattern();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "OLED test pattern failed: %s", esp_err_to_name(ret));
    }
    
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ret = oled_framebuffer_display_boot_info();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "OLED boot info failed: %s", esp_err_to_name(ret));
    }
    
    vTaskDelay(pdMS_TO_TICKS(3000));
}
#else
/**
 * @brief Show boot sequence via UART logs (Screenless Mode)
 */
static void show_boot_sequence(void) {
    ESP_LOGI(TAG, "LucidConsole v1.0 (Screenless)");
    vTaskDelay(pdMS_TO_TICKS(1000));
}
#endif

#if CONFIG_ENABLE_OLED_DISPLAY
/**
 * @brief FreeRTOS OLED Display Task (Production-Ready)
 * 
 * Dedicated task for all OLED display operations following Gemini's best practices:
 * - Thread-safe I2C access via mutex
 * - Proper watchdog handling with vTaskDelay()
 * - Stack usage monitoring for optimization
 * - Real-time system status display
 */
static void status_display_task(void* pvParameters) {
    uint32_t uptime_seconds = 0;
    uint32_t update_count = 0;
    
    
    while (true) {
        uptime_seconds = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
        update_count++;
        
        // Get WiFi status
        lucid_wifi_status_t wifi_status;
        wifi_manager_get_status(&wifi_status);
        
        // Prepare status data for framebuffer update
        oled_status_t status = {
            .uptime_sec = uptime_seconds,
            .free_heap = esp_get_free_heap_size(),
            .rx_count = uart_bridge_get_rx_count(),
            .tx_count = uart_bridge_get_tx_count(),
        };
        
        // Map WiFi manager state to display state
        switch (wifi_status.state) {
            case LUCID_WIFI_STATE_INIT:
                status.wifi_state = OLED_WIFI_INIT;
                break;
            case LUCID_WIFI_STATE_AP_MODE:
                status.wifi_state = OLED_WIFI_AP_MODE;
                break;
            case LUCID_WIFI_STATE_STA_CONNECTING:
                status.wifi_state = OLED_WIFI_CONNECTING;
                break;
            case LUCID_WIFI_STATE_STA_CONNECTED:
                status.wifi_state = OLED_WIFI_CONNECTED;
                break;
            case LUCID_WIFI_STATE_STA_DISCONNECTED:
                status.wifi_state = OLED_WIFI_DISCONNECTED;
                break;
            default:
                status.wifi_state = OLED_WIFI_INIT;
                break;
        }
        
        // Copy WiFi info (truncate SSID if too long for display)
        strncpy(status.wifi_ssid, wifi_status.ssid, sizeof(status.wifi_ssid) - 1);
        status.wifi_ssid[sizeof(status.wifi_ssid) - 1] = '\0';
        strncpy(status.ip_address, wifi_status.ip_address, sizeof(status.ip_address) - 1);
        status.ip_address[sizeof(status.ip_address) - 1] = '\0';
        status.rssi = wifi_status.rssi;
        status.sta_count = wifi_status.sta_count;
        
        // Single framebuffer update - smooth, no flicker!
        esp_err_t ret = oled_framebuffer_display_status(&status);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Framebuffer status update failed: %s", esp_err_to_name(ret));
        }
        
        
        // CRITICAL: vTaskDelay() prevents watchdog timeout (Gemini guidance)
        // Update every 1 second for responsive UI
        vTaskDelay(pdMS_TO_TICKS(CONFIG_DISPLAY_UPDATE_RATE_MS));
    }
}
#else
/**
 * @brief Status logging task (Screenless Mode)
 * 
 * Provides system status via UART logs instead of display
 */
static void status_logging_task(void* pvParameters) {
    uint32_t uptime_seconds = 0;
    uint32_t update_count = 0;
    
    while (true) {
        uptime_seconds = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
        update_count++;
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // Update every 1 second
    }
}
#endif

void app_main(void) {
    ESP_LOGI(TAG, "LucidConsole starting...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Core hardware initialization
    ESP_ERROR_CHECK(gpio_early_init());
    
    // Initialize WiFi manager
    ESP_ERROR_CHECK(wifi_manager_init());
    
    // Initialize web server
    ESP_ERROR_CHECK(web_server_init());
    
    // Initialize UART bridge system
    uart_bridge_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .echo_enabled = false,
        .timestamp_enabled = false
    };
    ESP_ERROR_CHECK(uart_bridge_init(&uart_config));
    ESP_ERROR_CHECK(uart_bridge_start());
    
    // Connect web server to UART bridge
    web_server_set_uart_callbacks(uart_bridge_get_rx_count, uart_bridge_get_tx_count);
    ESP_ERROR_CHECK(uart_bridge_set_rx_callback(web_server_broadcast_uart_data));
    
    #if CONFIG_ENABLE_OLED_DISPLAY
    // OLED initialization
    ESP_ERROR_CHECK(gpio_oled_power_on());
    ESP_ERROR_CHECK(i2c_hw_bus_init());
    
    int devices = i2c_hw_scan_devices();
    if (devices > 0) {
        ESP_ERROR_CHECK(oled_framebuffer_init());
        show_boot_sequence();
    } else {
        ESP_LOGW(TAG, "No I2C devices found, OLED disabled");
    }
    
    // Create OLED display task (Gemini optimized parameters)
    BaseType_t task_created = xTaskCreate(
        status_display_task,                          // Task function
        "oled_display",                               // Task name
        CONFIG_DISPLAY_TASK_STACK_SIZE,               // Stack: 8KB (2048 words)
        NULL,                                         // Task parameters  
        CONFIG_DISPLAY_TASK_PRIORITY,                 // Priority: tskIDLE_PRIORITY + 2
        NULL                                          // Task handle (not needed)
    );
    
    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Failed to create OLED display task");
        return;
    }
    #else
    
    // Show screenless boot sequence (UART logs only)
    show_boot_sequence();
    
    // Create status logging task (minimal overhead)
    BaseType_t task_created = xTaskCreate(
        status_logging_task,                          // Task function
        "status_log",                                 // Task name
        1024,                                         // Stack: 4KB (minimal)
        NULL,                                         // Task parameters
        tskIDLE_PRIORITY + 1,                         // Priority: Lower than display
        NULL                                          // Task handle
    );
    
    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Failed to create status logging task");
        return;
    }
    #endif
    
    ESP_LOGI(TAG, "Ready. Free heap: %u KB", esp_get_free_heap_size() / 1024);
}