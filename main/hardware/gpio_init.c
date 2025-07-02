/*
 * GPIO Initialization Module - ESP8266 Hardware-Specific Setup
 */

#include "gpio_init.h"
#include "driver/gpio.h"
#include "driver/spi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "GPIO_INIT";

esp_err_t gpio_early_init(void) {
    ESP_LOGI(TAG, "=== Early GPIO Initialization ===");
    
    // Step 1: Configure GPIO16 (RTC domain) for OLED power control
    ESP_LOGI(TAG, "Step 1: Configure GPIO16 (RTC domain) for OLED power...");
    gpio_config_t power_conf = {
        .pin_bit_mask = (1ULL << GPIO_OLED_POWER),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    esp_err_t ret = gpio_config(&power_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO16: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Set GPIO16 to OFF initially (1 = OFF, 0 = ON for this board)
    gpio_set_level(GPIO_OLED_POWER, 1);
    ESP_LOGI(TAG, "GPIO16 set to OFF (1) - OLED power disabled during init");
    
    // Step 2: I2C pins will be configured by hardware I2C driver
    ESP_LOGI(TAG, "Step 2: I2C pins (GPIO%d=SDA, GPIO%d=SCL) will be configured by hardware driver", 
             GPIO_I2C_SDA, GPIO_I2C_SCL);
    
    ESP_LOGI(TAG, "✅ Early GPIO initialization complete");
    ESP_LOGI(TAG, "=== I2C PINS RESERVED FOR HARDWARE DRIVER ===");
    
    return ESP_OK;
}

esp_err_t gpio_oled_power_on(void) {
    ESP_LOGI(TAG, "Turning OLED power ON...");
    
    // GPIO16 = 0 means OLED power ON for this board design
    gpio_set_level(GPIO_OLED_POWER, 0);
    
    // Power stabilization delay - critical for OLED init
    vTaskDelay(pdMS_TO_TICKS(50));
    
    ESP_LOGI(TAG, "✅ OLED power ON (GPIO16=0) - power rail stabilized");
    return ESP_OK;
}

esp_err_t gpio_oled_power_off(void) {
    ESP_LOGI(TAG, "Turning OLED power OFF...");
    
    // GPIO16 = 1 means OLED power OFF for this board design  
    gpio_set_level(GPIO_OLED_POWER, 1);
    
    ESP_LOGI(TAG, "✅ OLED power OFF (GPIO16=1)");
    return ESP_OK;
}

esp_err_t gpio_boot_button_init(void) {
    ESP_LOGI(TAG, "Initializing boot button (GPIO0)...");
    ESP_LOGW(TAG, "WARNING: GPIO0 is bootstrap pin - only call after WiFi init!");
    
    gpio_config_t button_conf = {
        .pin_bit_mask = (1ULL << GPIO_BOOT_BUTTON),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,  // Required for bootstrap pin
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,  // Configure interrupt separately if needed
    };
    
    esp_err_t ret = gpio_config(&button_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure boot button: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "✅ Boot button configured (GPIO0) with pullup");
    return ESP_OK;
}

bool gpio_boot_button_pressed(void) {
    // Button is active low (pressed = 0, released = 1)
    return gpio_get_level(GPIO_BOOT_BUTTON) == 0;
}