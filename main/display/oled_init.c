/*
 * OLED Initialization Module - SSD1306 Display Controller
 */

#include "oled_init.h"
#include "../bus/i2c_hw_bus.h"
#include "../hardware/gpio_init.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "OLED_INIT";

// SSD1306 Command Definitions
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_DISPLAYON           0xAF
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define SSD1306_SETMULTIPLEX        0xA8
#define SSD1306_SETDISPLAYOFFSET    0xD3
#define SSD1306_SETSTARTLINE        0x40
#define SSD1306_CHARGEPUMP          0x8D
#define SSD1306_MEMORYMODE          0x20
#define SSD1306_SEGREMAP            0xA1
#define SSD1306_COMSCANDEC          0xC8
#define SSD1306_SETCOMPINS          0xDA
#define SSD1306_SETCONTRAST         0x81
#define SSD1306_SETPRECHARGE        0xD9
#define SSD1306_SETVCOMDETECT       0xDB
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_INVERTDISPLAY       0xA7

esp_err_t oled_send_command(uint8_t cmd) {
    // Thread-safe SSD1306 command: ADDR → 0x00 → COMMAND
    esp_err_t ret = i2c_hw_bus_lock(1000); // 1 second timeout
    if (ret != ESP_OK) {
        return ret;
    }
    
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (OLED_I2C_ADDR << 1) | 0, true); // Write mode
    i2c_master_write_byte(i2c_cmd, 0x00, true); // Command mode
    i2c_master_write_byte(i2c_cmd, cmd, true);   // Command
    i2c_master_stop(i2c_cmd);
    
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, i2c_cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(i2c_cmd);
    
    i2c_hw_bus_unlock();
    return ret;
}

esp_err_t oled_send_data(uint8_t data_byte) {
    // Thread-safe SSD1306 data: ADDR → 0x40 → DATA
    esp_err_t ret = i2c_hw_bus_lock(1000); // 1 second timeout
    if (ret != ESP_OK) {
        return ret;
    }
    
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (OLED_I2C_ADDR << 1) | 0, true); // Write mode
    i2c_master_write_byte(i2c_cmd, 0x40, true); // Data mode
    i2c_master_write_byte(i2c_cmd, data_byte, true); // Data
    i2c_master_stop(i2c_cmd);
    
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, i2c_cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(i2c_cmd);
    
    i2c_hw_bus_unlock();
    return ret;
}

static esp_err_t oled_send_init_sequence(void) {
    ESP_LOGI(TAG, "Sending SSD1306 initialization sequence (Arduino-style)...");
    
    // Arduino Adafruit_SSD1306 initialization sequence for 128x64
    esp_err_t ret = ESP_OK;
    
    // Display OFF
    ret = oled_send_command(SSD1306_DISPLAYOFF);
    if (ret != ESP_OK) return ret;
    
    // Set display clock divide ratio/oscillator frequency
    ret = oled_send_command(SSD1306_SETDISPLAYCLOCKDIV);
    if (ret != ESP_OK) return ret;
    ret = oled_send_command(0x80); // Default ratio
    if (ret != ESP_OK) return ret;
    
    // Set multiplex ratio (1 to 64)
    ret = oled_send_command(SSD1306_SETMULTIPLEX);
    if (ret != ESP_OK) return ret;
    ret = oled_send_command(0x3F); // 128x64: 64-1
    if (ret != ESP_OK) return ret;
    
    // Set display offset (no offset)
    ret = oled_send_command(SSD1306_SETDISPLAYOFFSET);
    if (ret != ESP_OK) return ret;
    ret = oled_send_command(0x00);
    if (ret != ESP_OK) return ret;
    
    // Set start line address
    ret = oled_send_command(SSD1306_SETSTARTLINE | 0x0);
    if (ret != ESP_OK) return ret;
    
    // Charge pump
    ret = oled_send_command(SSD1306_CHARGEPUMP);
    if (ret != ESP_OK) return ret;
    ret = oled_send_command(0x14); // Enable charge pump
    if (ret != ESP_OK) return ret;
    
    // Memory addressing mode
    ret = oled_send_command(SSD1306_MEMORYMODE);
    if (ret != ESP_OK) return ret;
    ret = oled_send_command(0x00); // Horizontal addressing mode
    if (ret != ESP_OK) return ret;
    
    // Segment re-map (column 127 mapped to SEG0)
    ret = oled_send_command(SSD1306_SEGREMAP | 0x1);
    if (ret != ESP_OK) return ret;
    
    // Set COM output scan direction (remapped mode)
    ret = oled_send_command(SSD1306_COMSCANDEC);
    if (ret != ESP_OK) return ret;
    
    // Set COM pins hardware configuration for 128x64
    ret = oled_send_command(SSD1306_SETCOMPINS);
    if (ret != ESP_OK) return ret;
    ret = oled_send_command(0x12); // 128x64: Alternative COM pin config
    if (ret != ESP_OK) return ret;
    
    // Set contrast control (increased for visibility)
    ret = oled_send_command(SSD1306_SETCONTRAST);
    if (ret != ESP_OK) return ret;
    ret = oled_send_command(0xCF); // High contrast (was 0x7F)
    if (ret != ESP_OK) return ret;
    
    // Set pre-charge period
    ret = oled_send_command(SSD1306_SETPRECHARGE);
    if (ret != ESP_OK) return ret;
    ret = oled_send_command(0xF1); // Phase 1: 1 DCLK, Phase 2: 15 DCLKs
    if (ret != ESP_OK) return ret;
    
    // Set VCOMH deselect level
    ret = oled_send_command(SSD1306_SETVCOMDETECT);
    if (ret != ESP_OK) return ret;
    ret = oled_send_command(0x40); // VCOMH = 0.77*Vcc
    if (ret != ESP_OK) return ret;
    
    // Entire display ON (disable)
    ret = oled_send_command(SSD1306_DISPLAYALLON_RESUME);
    if (ret != ESP_OK) return ret;
    
    // Set normal display (font data already inverted for proper rendering)  
    ret = oled_send_command(SSD1306_NORMALDISPLAY); // 0xA6 - normal polarity
    if (ret != ESP_OK) return ret;
    
    // Display ON
    ret = oled_send_command(SSD1306_DISPLAYON);
    if (ret != ESP_OK) return ret;
    
    ESP_LOGI(TAG, "✅ Arduino-style SSD1306 initialization complete");
    return ESP_OK;
}

esp_err_t oled_test_hardware(void) {
    ESP_LOGI(TAG, "=== OLED Hardware Test ===");
    
    // Step 1: Turn on OLED power
    ESP_LOGI(TAG, "Step 1: Turn on OLED power...");
    esp_err_t ret = gpio_oled_power_on();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to turn on OLED power");
        return ret;
    }
    
    // Step 2: Test command sending (hardware I2C handles locking internally)
    ESP_LOGI(TAG, "Step 2: Test command sending...");
    ret = oled_send_command(SSD1306_DISPLAYOFF);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ OLED not detected or command failed");
        ESP_LOGE(TAG, "Check connections: SDA=GPIO%d, SCL=GPIO%d, Power=GPIO%d", 
                 I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, GPIO_OLED_POWER);
        return ret;
    }
    
    ESP_LOGI(TAG, "✅ OLED hardware test passed");
    return ESP_OK;
}

esp_err_t oled_clear_screen(void) {
    ESP_LOGI(TAG, "Clearing OLED screen...");
    
    // Set column address range (0 to 127)
    esp_err_t ret = oled_send_command(0x21);  // Set column address
    if (ret == ESP_OK) ret = oled_send_command(0x00);  // Column start
    if (ret == ESP_OK) ret = oled_send_command(0x7F);  // Column end
    
    // Set page address range (0 to 7)
    if (ret == ESP_OK) ret = oled_send_command(0x22);  // Set page address
    if (ret == ESP_OK) ret = oled_send_command(0x00);  // Page start
    if (ret == ESP_OK) ret = oled_send_command(0x07);  // Page end
    
    // Clear all pixels (send 0x00 for all 1024 bytes)
    if (ret == ESP_OK) {
        for (int page = 0; page < OLED_PAGES; page++) {
            for (int col = 0; col < OLED_WIDTH; col++) {
                ret = oled_send_data(0x00);
                if (ret != ESP_OK) break;
            }
            if (ret != ESP_OK) break;
            
            // Yield CPU periodically to avoid watchdog
            if (page % 2 == 0) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }
        }
    }
    
    // CRITICAL: Reset to horizontal addressing mode after clear
    if (ret == ESP_OK) {
        ret = oled_send_command(0x20); // Memory addressing mode
        if (ret == ESP_OK) ret = oled_send_command(0x00); // Back to horizontal
    }
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✅ OLED screen cleared and reset to horizontal mode");
    } else {
        ESP_LOGE(TAG, "Failed to clear OLED screen: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t oled_display_power(bool on) {
    ESP_LOGI(TAG, "Setting OLED display power: %s", on ? "ON" : "OFF");
    
    return oled_send_command(on ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF);
}

esp_err_t oled_init(void) {
    ESP_LOGI(TAG, "=== OLED Display Initialization ===");
    ESP_LOGI(TAG, "Display: 128x64 SSD1306 OLED");
    ESP_LOGI(TAG, "I2C Address: 0x%02X", OLED_I2C_ADDR);
    ESP_LOGI(TAG, "Pins: SDA=GPIO%d, SCL=GPIO%d, Power=GPIO%d", 
             I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, GPIO_OLED_POWER);
    
    // Step 1: Test hardware
    esp_err_t ret = oled_test_hardware();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ OLED hardware test failed");
        return ret;
    }
    
    // Step 2: Send initialization sequence
    ret = oled_send_init_sequence();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ OLED initialization sequence failed");
        return ret;
    }
    
    // Step 3: Clear screen
    ret = oled_clear_screen();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ Failed to clear OLED screen");
        return ret;
    }
    
    ESP_LOGI(TAG, "✅ OLED Display Initialization Complete");
    ESP_LOGI(TAG, "Display is ON and ready for rendering");
    
    return ESP_OK;
}