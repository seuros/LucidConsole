/*
 * OLED Render Module - Queue-Based Display Updates
 * 
 * Provides thread-safe render queue system for OLED updates.
 * Only the UI task draws to the display - other tasks post render commands.
 * 
 * Features:
 * - FreeRTOS queue-based render commands
 * - Simple text rendering with multiple lines  
 * - Non-blocking render posting from any task
 * - Dedicated UI task for display updates at controlled rate
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Render Configuration
#define RENDER_QUEUE_SIZE       16      // Maximum queued render commands
#define RENDER_MAX_TEXT_LEN     21      // Max chars per line (128px / 6px font)
#define RENDER_MAX_LINES        8       // OLED height / 8px per line

// UI Task Configuration  
#define UI_TASK_PRIORITY        2       // FreeRTOS priority
#define UI_TASK_STACK_SIZE      4096    // Stack size in bytes
#define UI_REFRESH_RATE_MS      50      // 20 FPS max refresh rate

/**
 * @brief Render command types
 */
typedef enum {
    RENDER_CMD_TEXT_LINE,       // Update single text line
    RENDER_CMD_CLEAR_SCREEN,    // Clear entire screen
    RENDER_CMD_DISPLAY_POWER,   // Turn display on/off
    RENDER_CMD_STATUS_UPDATE,   // Update system status display
} render_cmd_type_t;

/**
 * @brief Text line render command
 */
typedef struct {
    uint8_t line;                           // Line number (0-7)
    char text[RENDER_MAX_TEXT_LEN + 1];     // Text content (null terminated)
    bool clear_line;                        // Clear line before writing
} render_text_line_t;

/**
 * @brief Status update render command
 */
typedef struct {
    uint32_t rx_count;          // UART RX bytes
    uint32_t tx_count;          // UART TX bytes  
    uint32_t uptime_sec;        // System uptime
    uint32_t free_heap;         // Free heap bytes
    int8_t wifi_rssi;           // WiFi RSSI (dBm)
} render_status_t;

/**
 * @brief Render command structure
 */
typedef struct {
    render_cmd_type_t type;
    union {
        render_text_line_t text_line;
        render_status_t status;
        bool display_on;        // For RENDER_CMD_DISPLAY_POWER
    } data;
} render_cmd_t;

/**
 * @brief Initialize OLED render system
 * 
 * Creates render queue and starts UI task.
 * Must be called after OLED hardware initialization.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t oled_render_init(void);

/**
 * @brief Post text line render command (non-blocking)
 * 
 * Queues a text line update. If queue is full, command is dropped.
 * 
 * @param line Line number (0-7)
 * @param text Text to display (max 21 chars)
 * @param clear_line Clear line before writing
 * @return ESP_OK if queued, ESP_FAIL if queue full
 */
esp_err_t oled_render_text_line(uint8_t line, const char* text, bool clear_line);

/**
 * @brief Post clear screen command (non-blocking)
 * 
 * @return ESP_OK if queued, ESP_FAIL if queue full
 */
esp_err_t oled_render_clear_screen(void);

/**
 * @brief Post display power command (non-blocking)
 * 
 * @param on true to turn display on, false to turn off
 * @return ESP_OK if queued, ESP_FAIL if queue full
 */
esp_err_t oled_render_display_power(bool on);

/**
 * @brief Post status update command (non-blocking)
 * 
 * Updates system status display with current metrics.
 * 
 * @param status Pointer to status data
 * @return ESP_OK if queued, ESP_FAIL if queue full
 */
esp_err_t oled_render_status_update(const render_status_t* status);

/**
 * @brief Get render queue statistics
 * 
 * @param queued_items Number of items currently in queue
 * @param dropped_items Total number of dropped items (queue full)
 */
void oled_render_get_stats(uint32_t* queued_items, uint32_t* dropped_items);

/**
 * @brief Render test pattern to isolate font rendering issues
 * 
 * Displays horizontal stripes to verify basic rendering works.
 * If stripes show clean, problem is font data. If garbled, I2C/init issue.
 * 
 * @return ESP_OK on success
 */
esp_err_t oled_render_test_pattern(void);

#ifdef __cplusplus
}
#endif