/*
 * OLED Framebuffer Render Module - Eliminates Oscilloscope Flicker
 * Uses SSD1306 library's framebuffer for smooth display updates
 */

#include "oled_framebuffer.h"
#include "../bus/i2c_hw_bus.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>

// Include SSD1306 library and fonts
#include "ssd1306/ssd1306.h"
#include "fonts/fonts.h"

static const char* TAG = "OLED_FB";

// SSD1306 device instance
static ssd1306_t ssd1306_dev;

// Framebuffer for 128x64 display (1024 bytes)
static uint8_t framebuffer[OLED_FB_SIZE];

// Font selection - use small GLCD font for terminal-like display
extern const font_info_t _fonts_glcd_5x7_info;

esp_err_t oled_framebuffer_init(void) {
    ESP_LOGI(TAG, "Initializing SSD1306 framebuffer system");
    
    // Configure SSD1306 device
    ssd1306_dev.i2c_port = I2C_MASTER_NUM;
    ssd1306_dev.i2c_addr = OLED_I2C_ADDR;
    ssd1306_dev.screen = SSD1306_SCREEN;
    ssd1306_dev.width = OLED_WIDTH;
    ssd1306_dev.height = OLED_HEIGHT;
    
    // Initialize SSD1306 display
    int res = ssd1306_init(&ssd1306_dev);
    if (res != 0) {
        ESP_LOGE(TAG, "SSD1306 init failed: %d", res);
        return ESP_FAIL;
    }
    
    // Clear framebuffer
    memset(framebuffer, 0, sizeof(framebuffer));
    
    // Upload initial clear framebuffer
    res = ssd1306_load_frame_buffer(&ssd1306_dev, framebuffer);
    if (res != 0) {
        ESP_LOGE(TAG, "Failed to load framebuffer: %d", res);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "SSD1306 framebuffer system initialized");
    return ESP_OK;
}

esp_err_t oled_framebuffer_clear(void) {
    // Clear local framebuffer
    memset(framebuffer, 0, sizeof(framebuffer));
    return ESP_OK;
}

esp_err_t oled_framebuffer_update(void) {
    // Upload framebuffer to display - single I2C transaction
    int res = ssd1306_load_frame_buffer(&ssd1306_dev, framebuffer);
    if (res != 0) {
        ESP_LOGE(TAG, "Failed to update display: %d", res);
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t oled_framebuffer_draw_text(uint8_t x, uint8_t y, const char* text) {
    if (!text) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Draw string to framebuffer using SSD1306 library
    int res = ssd1306_draw_string(&ssd1306_dev, framebuffer, &_fonts_glcd_5x7_info, 
                                  x, y, text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    if (res < 0) {
        ESP_LOGW(TAG, "Failed to draw text: %d", res);
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t oled_framebuffer_draw_text_line(uint8_t line, const char* text, bool clear_line) {
    if (line >= OLED_MAX_LINES || !text) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t y = line * OLED_LINE_HEIGHT;
    
    // Clear line area if requested
    if (clear_line) {
        // Fill rectangle with black (clear)
        int res = ssd1306_fill_rectangle(&ssd1306_dev, framebuffer, 
                                       0, y, OLED_WIDTH, OLED_LINE_HEIGHT, 
                                       OLED_COLOR_BLACK);
        if (res != 0) {
            ESP_LOGW(TAG, "Failed to clear line: %d", res);
        }
    }
    
    // Draw text on line
    return oled_framebuffer_draw_text(0, y, text);
}

esp_err_t oled_framebuffer_render_test_pattern(void) {
    ESP_LOGI(TAG, "Rendering test pattern to framebuffer");
    
    // Clear framebuffer
    oled_framebuffer_clear();
    
    // Draw alternating stripes 
    for (int y = 0; y < OLED_HEIGHT; y += 2) {
        ssd1306_draw_hline(&ssd1306_dev, framebuffer, 0, y, OLED_WIDTH, OLED_COLOR_WHITE);
    }
    
    // Update display with single transaction
    return oled_framebuffer_update();
}

esp_err_t oled_framebuffer_display_boot_info(void) {
    ESP_LOGI(TAG, "Displaying boot information");
    
    // Clear framebuffer
    oled_framebuffer_clear();
    
    // Draw boot information
    oled_framebuffer_draw_text_line(0, "LucidConsole v1.0", false);
    oled_framebuffer_draw_text_line(1, "ESP8266 UART Bridge", false);  
    oled_framebuffer_draw_text_line(2, "Framebuffer Ready", false);
    oled_framebuffer_draw_text_line(3, "No More Flicker!", false);
    
    // Single update to display
    return oled_framebuffer_update();
}

esp_err_t oled_framebuffer_display_status(const oled_status_t* status) {
    if (!status) {
        return ESP_ERR_INVALID_ARG;
    }
    
    char line_buf[32];
    
    // Clear framebuffer
    oled_framebuffer_clear();
    
    // Line 0: Title + Uptime
    snprintf(line_buf, sizeof(line_buf), "LucidConsole %us", status->uptime_sec);
    oled_framebuffer_draw_text_line(0, line_buf, false);
    
    // Line 1: WiFi Status
    switch (status->wifi_state) {
        case OLED_WIFI_INIT:
            snprintf(line_buf, sizeof(line_buf), "WiFi: Initializing");
            break;
        case OLED_WIFI_AP_MODE:
            snprintf(line_buf, sizeof(line_buf), "AP: %s", status->wifi_ssid);
            break;
        case OLED_WIFI_CONNECTING:
            snprintf(line_buf, sizeof(line_buf), "Conn: %s", status->wifi_ssid);
            break;
        case OLED_WIFI_CONNECTED:
            snprintf(line_buf, sizeof(line_buf), "STA: %s %ddBm", status->wifi_ssid, status->rssi);
            break;
        case OLED_WIFI_DISCONNECTED:
            snprintf(line_buf, sizeof(line_buf), "WiFi: Disconnected");
            break;
        default:
            snprintf(line_buf, sizeof(line_buf), "WiFi: Unknown");
            break;
    }
    oled_framebuffer_draw_text_line(1, line_buf, false);
    
    // Line 2: IP Address or Client Count
    if (status->wifi_state == OLED_WIFI_AP_MODE) {
        snprintf(line_buf, sizeof(line_buf), "%s (%u client%s)", 
                status->ip_address, status->sta_count, 
                status->sta_count == 1 ? "" : "s");
    } else {
        snprintf(line_buf, sizeof(line_buf), "IP: %s", status->ip_address);
    }
    oled_framebuffer_draw_text_line(2, line_buf, false);
    
    // Line 3: Memory + UART Stats
    snprintf(line_buf, sizeof(line_buf), "%uK RX:%u TX:%u", 
            status->free_heap / 1024, status->rx_count, status->tx_count);
    oled_framebuffer_draw_text_line(3, line_buf, false);
    
    // Single update to display - smooth, no flicker!
    return oled_framebuffer_update();
}