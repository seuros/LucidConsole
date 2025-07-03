/*
 * UART Bridge - LucidConsole Serial Communication Bridge Implementation
 * Handles bidirectional data flow between UART and network clients
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "uart_bridge.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <string.h>

static const char* TAG = "UART_BRIDGE";

// Bridge state
static bool bridge_initialized = false;
static bool bridge_active = false;
static uart_bridge_config_t current_config;
static uart_bridge_stats_t bridge_stats = {0};

// FreeRTOS resources
static TaskHandle_t uart_rx_task_handle = NULL;
static QueueHandle_t uart_event_queue = NULL;

// Data callback for forwarding UART data to network clients
static void (*rx_data_callback)(const uint8_t* data, size_t length) = NULL;

// Internal buffers
static uint8_t rx_buffer[LUCIDUART_RX_BUF_SIZE];

/**
 * @brief UART event handling task
 * 
 * Processes UART events (RX data, errors, etc.) and forwards data
 * to registered callback (typically WebSocket server).
 */
static void uart_event_task(void* pvParameters) {
    uart_event_t event;
    size_t buffered_size;
    
    ESP_LOGI(TAG, "UART event task started");
    bridge_stats.bridge_uptime = 0;
    
    while (bridge_active) {
        // Wait for UART event
        if (xQueueReceive(uart_event_queue, (void*)&event, pdMS_TO_TICKS(100))) {
            switch (event.type) {
                case UART_DATA:
                    // Data received from UART - forward to network clients
                    uart_get_buffered_data_len(LUCIDUART_UART_NUM, &buffered_size);
                    if (buffered_size > 0) {
                        size_t bytes_to_read = (buffered_size < sizeof(rx_buffer)) ? 
                                               buffered_size : sizeof(rx_buffer);
                        
                        int bytes_read = uart_read_bytes(LUCIDUART_UART_NUM, rx_buffer, 
                                                        bytes_to_read, pdMS_TO_TICKS(100));
                        
                        if (bytes_read > 0) {
                            bridge_stats.rx_bytes += bytes_read;
                            
                            // Forward to network clients via callback
                            if (rx_data_callback) {
                                rx_data_callback(rx_buffer, bytes_read);
                            }
                            
                            ESP_LOGD(TAG, "UART RX: %d bytes (total: %u)", 
                                    bytes_read, bridge_stats.rx_bytes);
                        }
                    }
                    break;
                    
                case UART_FIFO_OVF:
                    ESP_LOGW(TAG, "UART FIFO overflow");
                    bridge_stats.rx_errors++;
                    uart_flush_input(LUCIDUART_UART_NUM);
                    xQueueReset(uart_event_queue);
                    break;
                    
                case UART_BUFFER_FULL:
                    ESP_LOGW(TAG, "UART ring buffer full");
                    bridge_stats.rx_errors++;
                    uart_flush_input(LUCIDUART_UART_NUM);
                    xQueueReset(uart_event_queue);
                    break;
                    
                // Note: UART_BREAK not available in ESP8266 SDK
                // case UART_BREAK:
                //     ESP_LOGD(TAG, "UART break detected");
                //     break;
                    
                case UART_PARITY_ERR:
                    ESP_LOGW(TAG, "UART parity error");
                    bridge_stats.rx_errors++;
                    break;
                    
                case UART_FRAME_ERR:
                    ESP_LOGW(TAG, "UART frame error");
                    bridge_stats.rx_errors++;
                    break;
                    
                default:
                    ESP_LOGD(TAG, "UART event: %d", event.type);
                    break;
            }
        }
        
        // Update uptime counter
        static uint32_t uptime_counter = 0;
        if (++uptime_counter >= 10) {  // Every ~1 second (100ms * 10)
            bridge_stats.bridge_uptime++;
            uptime_counter = 0;
        }
    }
    
    ESP_LOGI(TAG, "UART event task ended");
    vTaskDelete(NULL);
}

esp_err_t uart_bridge_init(const uart_bridge_config_t* config) {
    if (bridge_initialized) {
        ESP_LOGW(TAG, "UART bridge already initialized");
        return ESP_OK;
    }
    
    // Initialize UART bridge
    
    // Set default configuration if none provided
    if (config) {
        current_config = *config;
    } else {
        current_config = (uart_bridge_config_t){
            .baud_rate = LUCIDUART_DEFAULT_BAUD,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .echo_enabled = false,
            .timestamp_enabled = false
        };
    }
    
    // UART configuration (ESP8266 compatible)
    uart_config_t uart_config = {
        .baud_rate = current_config.baud_rate,
        .data_bits = current_config.data_bits,
        .parity = current_config.parity,
        .stop_bits = current_config.stop_bits,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        // Note: source_clk not available in ESP8266 SDK
    };
    
    // Install UART driver
    esp_err_t ret = uart_driver_install(LUCIDUART_UART_NUM, 
                                        LUCIDUART_RX_BUF_SIZE, 
                                        LUCIDUART_TX_BUF_SIZE, 
                                        LUCIDUART_QUEUE_SIZE, 
                                        &uart_event_queue, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Configure UART parameters
    ret = uart_param_config(LUCIDUART_UART_NUM, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UART: %s", esp_err_to_name(ret));
        uart_driver_delete(LUCIDUART_UART_NUM);
        return ret;
    }
    
    // Note: ESP8266 UART0 pins are fixed (GPIO1=TX, GPIO3=RX)
    // uart_set_pin() is not available in ESP8266 SDK
    
    // Reset statistics
    memset(&bridge_stats, 0, sizeof(bridge_stats));
    bridge_stats.current_baud = current_config.baud_rate;
    
    bridge_initialized = true;
    ESP_LOGI(TAG, "UART bridge initialized (baud: %u, pins: TX=%d RX=%d)", 
             current_config.baud_rate, LUCIDUART_TX_PIN, LUCIDUART_RX_PIN);
    
    return ESP_OK;
}

esp_err_t uart_bridge_start(void) {
    if (!bridge_initialized) {
        ESP_LOGE(TAG, "Bridge not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (bridge_active) {
        ESP_LOGW(TAG, "Bridge already active");
        return ESP_OK;
    }
    
    // Start UART bridge
    
    bridge_active = true;
    bridge_stats.bridge_active = true;
    
    // Create UART event handling task
    BaseType_t task_created = xTaskCreate(uart_event_task, 
                                         "uart_rx_task", 
                                         4096,  // Stack size
                                         NULL, 
                                         5,     // Priority
                                         &uart_rx_task_handle);
    
    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Failed to create UART event task");
        bridge_active = false;
        bridge_stats.bridge_active = false;
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "UART bridge started - ready for data transfer");
    return ESP_OK;
}

esp_err_t uart_bridge_stop(void) {
    if (!bridge_active) {
        ESP_LOGW(TAG, "Bridge not active");
        return ESP_OK;
    }
    
    // Stop UART bridge
    
    bridge_active = false;
    bridge_stats.bridge_active = false;
    
    // Wait for task to terminate
    if (uart_rx_task_handle) {
        // Task will self-delete when bridge_active becomes false
        uart_rx_task_handle = NULL;
    }
    
    ESP_LOGI(TAG, "UART bridge stopped");
    return ESP_OK;
}

esp_err_t uart_bridge_deinit(void) {
    if (!bridge_initialized) {
        ESP_LOGW(TAG, "Bridge not initialized");
        return ESP_OK;
    }
    
    // Stop bridge if active
    uart_bridge_stop();
    
    // Deinitialize UART bridge
    
    // Delete UART driver
    esp_err_t ret = uart_driver_delete(LUCIDUART_UART_NUM);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to delete UART driver: %s", esp_err_to_name(ret));
    }
    
    uart_event_queue = NULL;
    rx_data_callback = NULL;
    bridge_initialized = false;
    
    ESP_LOGI(TAG, "UART bridge deinitialized");
    return ESP_OK;
}

int uart_bridge_send(const uint8_t* data, size_t length) {
    if (!bridge_active || !data || length == 0) {
        return -1;
    }
    
    int bytes_sent = uart_write_bytes(LUCIDUART_UART_NUM, (const char*)data, length);
    if (bytes_sent > 0) {
        bridge_stats.tx_bytes += bytes_sent;
        ESP_LOGD(TAG, "UART TX: %d bytes (total: %u)", bytes_sent, bridge_stats.tx_bytes);
    } else {
        bridge_stats.tx_errors++;
        ESP_LOGW(TAG, "UART TX failed");
    }
    
    return bytes_sent;
}

esp_err_t uart_bridge_set_rx_callback(void (*callback)(const uint8_t* data, size_t length)) {
    rx_data_callback = callback;
    ESP_LOGI(TAG, "RX callback %s", callback ? "registered" : "cleared");
    return ESP_OK;
}

esp_err_t uart_bridge_get_stats(uart_bridge_stats_t* stats) {
    if (!stats) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *stats = bridge_stats;
    return ESP_OK;
}

esp_err_t uart_bridge_reset_stats(void) {
    bridge_stats.rx_bytes = 0;
    bridge_stats.tx_bytes = 0;
    bridge_stats.rx_errors = 0;
    bridge_stats.tx_errors = 0;
    bridge_stats.bridge_uptime = 0;
    
    ESP_LOGI(TAG, "Statistics reset");
    return ESP_OK;
}

esp_err_t uart_bridge_update_config(const uart_bridge_config_t* config) {
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Updating UART configuration (baud: %u)", config->baud_rate);
    
    bool was_active = bridge_active;
    
    // Stop bridge if active
    if (was_active) {
        uart_bridge_stop();
        vTaskDelay(pdMS_TO_TICKS(100)); // Allow task to terminate
    }
    
    // Update configuration
    current_config = *config;
    bridge_stats.current_baud = config->baud_rate;
    
    // Apply new UART parameters (ESP8266 compatible)
    uart_config_t uart_config = {
        .baud_rate = config->baud_rate,
        .data_bits = config->data_bits,
        .parity = config->parity,
        .stop_bits = config->stop_bits,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        // Note: source_clk not available in ESP8266 SDK
    };
    
    esp_err_t ret = uart_param_config(LUCIDUART_UART_NUM, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to update UART config: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Restart bridge if it was active
    if (was_active) {
        ret = uart_bridge_start();
    }
    
    ESP_LOGI(TAG, "UART configuration updated");
    return ret;
}

bool uart_bridge_is_active(void) {
    return bridge_active;
}

esp_err_t uart_bridge_gpio_control(bool tx_enable, bool rx_enable) {
    ESP_LOGI(TAG, "GPIO control: TX=%s, RX=%s", 
             tx_enable ? "enabled" : "disabled",
             rx_enable ? "enabled" : "disabled");
    
    if (!tx_enable) {
        // Tri-state TX pin
        gpio_set_direction(LUCIDUART_TX_PIN, GPIO_MODE_INPUT);
        gpio_set_pull_mode(LUCIDUART_TX_PIN, GPIO_FLOATING);
    } else {
        // Re-enable TX as UART output
        // Note: ESP8266 UART0 pins are fixed, no need to set pins
    }
    
    // RX control is handled by enabling/disabling UART receive
    // (Implementation depends on specific requirements)
    
    return ESP_OK;
}

uint32_t uart_bridge_get_rx_count(void) {
    return bridge_stats.rx_bytes;
}

uint32_t uart_bridge_get_tx_count(void) {
    return bridge_stats.tx_bytes;
}