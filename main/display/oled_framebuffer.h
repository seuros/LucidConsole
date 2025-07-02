/*
 * OLED Framebuffer Render Module - Header
 * Smooth display updates using SSD1306 library framebuffer
 */

#pragma once

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

// Display parameters
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_PAGES          8
#define OLED_FB_SIZE        (OLED_WIDTH * OLED_HEIGHT / 8)  // 1024 bytes
#define OLED_I2C_ADDR       0x3C

// Text layout parameters
#define OLED_LINE_HEIGHT    8     // 8 pixels per text line
#define OLED_MAX_LINES      (OLED_HEIGHT / OLED_LINE_HEIGHT)  // 8 lines max

// I2C configuration (from i2c_hw_bus.h) - use I2C_MASTER_NUM from i2c_hw_bus.h

// WiFi status modes for display
typedef enum {
    OLED_WIFI_INIT = 0,
    OLED_WIFI_AP_MODE,
    OLED_WIFI_CONNECTING,
    OLED_WIFI_CONNECTED,
    OLED_WIFI_DISCONNECTED
} oled_wifi_state_t;

// Status structure for display updates
typedef struct {
    uint32_t uptime_sec;
    uint32_t free_heap;
    
    // WiFi status
    oled_wifi_state_t wifi_state;
    char wifi_ssid[17];     // Truncated SSID for display (16 chars + null)
    char ip_address[16];    // IP address string
    int8_t rssi;           // Signal strength (-100 to 0)
    uint8_t sta_count;     // Connected clients (AP mode)
    
    // UART bridge status
    uint32_t rx_count;
    uint32_t tx_count;
} oled_status_t;

/**
 * @brief Initialize SSD1306 framebuffer system
 * 
 * Sets up the SSD1306 device and allocates framebuffer.
 * Must be called after I2C bus initialization.
 * 
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t oled_framebuffer_init(void);

/**
 * @brief Clear local framebuffer
 * 
 * Clears the local framebuffer but does not update display.
 * Call oled_framebuffer_update() to show changes.
 * 
 * @return ESP_OK on success
 */
esp_err_t oled_framebuffer_clear(void);

/**
 * @brief Update display with current framebuffer content
 * 
 * Uploads the entire framebuffer to display in a single I2C transaction.
 * This eliminates flicker and provides smooth updates.
 * 
 * @return ESP_OK on success, ESP_FAIL on I2C error
 */
esp_err_t oled_framebuffer_update(void);

/**
 * @brief Draw text at specific pixel coordinates
 * 
 * Draws text to framebuffer at given x,y coordinates.
 * Does not update display - call oled_framebuffer_update().
 * 
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-63)
 * @param text Text string to draw
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG on invalid parameters
 */
esp_err_t oled_framebuffer_draw_text(uint8_t x, uint8_t y, const char* text);

/**
 * @brief Draw text on specific line (8-pixel high lines)
 * 
 * Draws text on a specific text line (0-7).
 * Each line is 8 pixels high for optimal readability.
 * 
 * @param line Line number (0-7)
 * @param text Text string to draw
 * @param clear_line Clear line before drawing
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG on invalid parameters
 */
esp_err_t oled_framebuffer_draw_text_line(uint8_t line, const char* text, bool clear_line);

/**
 * @brief Render test pattern for diagnostics
 * 
 * Draws horizontal stripes to test framebuffer functionality.
 * Updates display immediately.
 * 
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t oled_framebuffer_render_test_pattern(void);

/**
 * @brief Display boot information
 * 
 * Shows boot status and framebuffer ready message.
 * Updates display immediately.
 * 
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t oled_framebuffer_display_boot_info(void);

/**
 * @brief Display system status information
 * 
 * Shows uptime, memory, WiFi status in clean layout.
 * Updates display immediately with single transaction.
 * 
 * @param status Pointer to status structure
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if status is NULL
 */
esp_err_t oled_framebuffer_display_status(const oled_status_t* status);