/*
 * OLED Render Module - Queue-Based Display Updates
 */

#include "oled_render.h"
#include "oled_init.h"
#include "../bus/i2c_hw_bus.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include <stdio.h>

static const char* TAG = "OLED_RENDER";

// Render system state
static QueueHandle_t render_queue = NULL;
static TaskHandle_t ui_task_handle = NULL;
static uint32_t dropped_commands = 0;

// Clean 6×8 monospace font - 5 pixels wide + 1 spacer column
// Standard orientation for SSD1306 (may need ~ inversion in send_data)
static const uint8_t font_6x8[95][6] = {
    {0x00,0x00,0x00,0x00,0x00,0x00}, // ' ' (space)
    {0x00,0x00,0x4F,0x00,0x00,0x00}, // '!'
    {0x00,0x07,0x00,0x07,0x00,0x00}, // '"'
    {0x14,0x7F,0x14,0x7F,0x14,0x00}, // '#'
    {0x24,0x2A,0x7F,0x2A,0x12,0x00}, // '$'
    {0x23,0x13,0x08,0x64,0x62,0x00}, // '%'
    {0x36,0x49,0x55,0x22,0x50,0x00}, // '&'
    {0x00,0x05,0x03,0x00,0x00,0x00}, // '''
    {0x00,0x1C,0x22,0x41,0x00,0x00}, // '('
    {0x00,0x41,0x22,0x1C,0x00,0x00}, // ')'
    {0x14,0x08,0x3E,0x08,0x14,0x00}, // '*'
    {0x08,0x08,0x3E,0x08,0x08,0x00}, // '+'
    {0x00,0x50,0x30,0x00,0x00,0x00}, // ','
    {0x08,0x08,0x08,0x08,0x08,0x00}, // '-'
    {0x00,0x60,0x60,0x00,0x00,0x00}, // '.'
    {0x20,0x10,0x08,0x04,0x02,0x00}, // '/'
    {0x3E,0x51,0x49,0x45,0x3E,0x00}, // '0'
    {0x00,0x42,0x7F,0x40,0x00,0x00}, // '1'
    {0x42,0x61,0x51,0x49,0x46,0x00}, // '2'
    {0x21,0x41,0x45,0x4B,0x31,0x00}, // '3'
    {0x18,0x14,0x12,0x7F,0x10,0x00}, // '4'
    {0x27,0x45,0x45,0x45,0x39,0x00}, // '5'
    {0x3C,0x4A,0x49,0x49,0x30,0x00}, // '6'
    {0x01,0x71,0x09,0x05,0x03,0x00}, // '7'
    {0x36,0x49,0x49,0x49,0x36,0x00}, // '8'
    {0x06,0x49,0x49,0x29,0x1E,0x00}, // '9'
    {0x00,0x36,0x36,0x00,0x00,0x00}, // ':'
    {0x00,0x56,0x36,0x00,0x00,0x00}, // ';'
    {0x08,0x14,0x22,0x41,0x00,0x00}, // '<'
    {0x14,0x14,0x14,0x14,0x14,0x00}, // '='
    {0x00,0x41,0x22,0x14,0x08,0x00}, // '>'
    {0x02,0x01,0x51,0x09,0x06,0x00}, // '?'
    {0x32,0x49,0x59,0x51,0x3E,0x00}, // '@'
    {0x7E,0x11,0x11,0x11,0x7E,0x00}, // 'A'
    {0x7F,0x49,0x49,0x49,0x36,0x00}, // 'B'
    {0x3E,0x41,0x41,0x41,0x22,0x00}, // 'C'
    {0x7F,0x41,0x41,0x22,0x1C,0x00}, // 'D'
    {0x7F,0x49,0x49,0x49,0x41,0x00}, // 'E'
    {0x7F,0x09,0x09,0x09,0x01,0x00}, // 'F'
    {0x3E,0x41,0x49,0x49,0x7A,0x00}, // 'G'
    {0x7F,0x08,0x08,0x08,0x7F,0x00}, // 'H'
    {0x00,0x41,0x7F,0x41,0x00,0x00}, // 'I'
    {0x20,0x40,0x41,0x3F,0x01,0x00}, // 'J'
    {0x7F,0x08,0x14,0x22,0x41,0x00}, // 'K'
    {0x7F,0x40,0x40,0x40,0x40,0x00}, // 'L'
    {0x7F,0x02,0x04,0x02,0x7F,0x00}, // 'M'
    {0x7F,0x04,0x08,0x10,0x7F,0x00}, // 'N'
    {0x3E,0x41,0x41,0x41,0x3E,0x00}, // 'O'
    {0x7F,0x09,0x09,0x09,0x06,0x00}, // 'P'
    {0x3E,0x41,0x51,0x21,0x5E,0x00}, // 'Q'
    {0x7F,0x09,0x19,0x29,0x46,0x00}, // 'R'
    {0x46,0x49,0x49,0x49,0x31,0x00}, // 'S'
    {0x01,0x01,0x7F,0x01,0x01,0x00}, // 'T'
    {0x3F,0x40,0x40,0x40,0x3F,0x00}, // 'U'
    {0x1F,0x20,0x40,0x20,0x1F,0x00}, // 'V'
    {0x3F,0x40,0x38,0x40,0x3F,0x00}, // 'W'
    {0x63,0x14,0x08,0x14,0x63,0x00}, // 'X'
    {0x07,0x08,0x70,0x08,0x07,0x00}, // 'Y'
    {0x61,0x51,0x49,0x45,0x43,0x00}, // 'Z'
    {0x00,0x7F,0x41,0x41,0x00,0x00}, // '['
    {0x02,0x04,0x08,0x10,0x20,0x00}, // '\'
    {0x00,0x41,0x41,0x7F,0x00,0x00}, // ']'
    {0x04,0x02,0x01,0x02,0x04,0x00}, // '^'
    {0x40,0x40,0x40,0x40,0x40,0x00}, // '_'
    {0x00,0x01,0x02,0x04,0x00,0x00}, // '`'
    // Proper lowercase a-z (your clean set)
    {0x20,0x54,0x54,0x54,0x78,0x00}, // a
    {0x7F,0x48,0x44,0x44,0x38,0x00}, // b
    {0x38,0x44,0x44,0x44,0x20,0x00}, // c
    {0x38,0x44,0x44,0x48,0x7F,0x00}, // d
    {0x38,0x54,0x54,0x54,0x18,0x00}, // e
    {0x08,0x7E,0x09,0x01,0x02,0x00}, // f
    {0x0C,0x52,0x52,0x52,0x3E,0x00}, // g
    {0x7F,0x08,0x04,0x04,0x78,0x00}, // h
    {0x00,0x44,0x7D,0x40,0x00,0x00}, // i
    {0x20,0x40,0x44,0x3D,0x00,0x00}, // j
    {0x7F,0x10,0x28,0x44,0x00,0x00}, // k
    {0x00,0x41,0x7F,0x40,0x00,0x00}, // l
    {0x7C,0x04,0x18,0x04,0x78,0x00}, // m
    {0x7C,0x08,0x04,0x04,0x78,0x00}, // n
    {0x38,0x44,0x44,0x44,0x38,0x00}, // o
    {0x7C,0x14,0x14,0x14,0x08,0x00}, // p
    {0x08,0x14,0x14,0x08,0x7C,0x00}, // q
    {0x7C,0x08,0x04,0x04,0x08,0x00}, // r
    {0x48,0x54,0x54,0x54,0x20,0x00}, // s
    {0x04,0x3F,0x44,0x40,0x20,0x00}, // t
    {0x3C,0x40,0x40,0x20,0x7C,0x00}, // u
    {0x1C,0x20,0x40,0x20,0x1C,0x00}, // v
    {0x3C,0x40,0x30,0x40,0x3C,0x00}, // w
    {0x44,0x28,0x10,0x28,0x44,0x00}, // x
    {0x0C,0x50,0x50,0x50,0x3C,0x00}, // y
    {0x44,0x64,0x54,0x4C,0x44,0x00}, // z
    {0x00,0x08,0x36,0x41,0x00,0x00}, // '{'
    {0x00,0x00,0x7F,0x00,0x00,0x00}, // '|'
    {0x00,0x41,0x36,0x08,0x00,0x00}, // '}'
    {0x10,0x08,0x08,0x10,0x08,0x00}, // '~'
};

/**
 * @brief Draw a character at specific position
 * 
 * @param x X position (0-127)
 * @param y Y position (page 0-7)  
 * @param c Character to draw
 * @return ESP_OK on success
 */
// Test pattern function to isolate garbling
esp_err_t oled_render_test_pattern(void) {
    ESP_LOGI(TAG, "Rendering test pattern to isolate font issues...");
    esp_err_t ret = oled_clear_screen();
    if (ret != ESP_OK) return ret;
    
    // Fill alternating stripes across all pages
    for (int page = 0; page < 8; page++) {
        ret = oled_send_command(0x22);  // Page range
        if (ret == ESP_OK) ret = oled_send_command(page);
        if (ret == ESP_OK) ret = oled_send_command(page);
        if (ret == ESP_OK) ret = oled_send_command(0x21);  // Column range
        if (ret == ESP_OK) ret = oled_send_command(0);
        if (ret == ESP_OK) ret = oled_send_command(127);
        
        uint8_t pattern = (page % 2 == 0) ? 0xAA : 0x55;  // Alternating bits
        for (int col = 0; col < 128; col++) {
            ret = oled_send_data(pattern);
            if (ret != ESP_OK) return ret;
        }
        vTaskDelay(1); // Yield to prevent watchdog
    }
    
    // Reset to horizontal addressing mode after pattern
    ret = oled_send_command(0x20); // Memory addressing mode
    if (ret == ESP_OK) ret = oled_send_command(0x00); // Horizontal
    
    ESP_LOGI(TAG, "Test pattern complete - should show horizontal stripes");
    return ret;
}

static esp_err_t draw_char(uint8_t x, uint8_t y, char c) {
    if (x >= OLED_WIDTH || y >= OLED_PAGES) {
        return ESP_ERR_INVALID_ARG;
    }
    
    
    // Set addressing to horizontal mode first
    esp_err_t ret = oled_send_command(0x20); // Memory addressing mode  
    if (ret == ESP_OK) ret = oled_send_command(0x00); // Horizontal
    
    // Set cursor position with proper addressing
    if (ret == ESP_OK) ret = oled_send_command(0x21);  // Column address
    if (ret == ESP_OK) ret = oled_send_command(x);
    if (ret == ESP_OK) ret = oled_send_command(x + 5);  // 6 pixels wide (0-5)
    if (ret == ESP_OK) ret = oled_send_command(0x22);  // Page address  
    if (ret == ESP_OK) ret = oled_send_command(y);
    if (ret == ESP_OK) ret = oled_send_command(y);
    
    // Character mapping: ASCII 32-126 maps to font indices 0-94
    uint8_t char_index = (c < 32 || c > 126) ? 0 : (c - 32);
    
    // Send character data (no inversion - font is properly oriented)
    for (int i = 0; i < 6 && ret == ESP_OK; i++) {
        ret = oled_send_data(font_6x8[char_index][i]);
    }
    
    return ret;
}

/**
 * @brief Render text on a specific line
 * 
 * @param line Line number (0-7)
 * @param text Text to render
 * @param clear_line Clear line before writing
 * @return ESP_OK on success
 */
static esp_err_t render_text_line_impl(uint8_t line, const char* text, bool clear_line) {
    if (line >= RENDER_MAX_LINES || text == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ESP_OK;
    
    // Clear line if requested
    if (clear_line) {
        // Set address range for entire line
        ret = oled_send_command(0x21);  // Column address
        if (ret == ESP_OK) ret = oled_send_command(0);
        if (ret == ESP_OK) ret = oled_send_command(127);
        if (ret == ESP_OK) ret = oled_send_command(0x22);  // Page address
        if (ret == ESP_OK) ret = oled_send_command(line);
        if (ret == ESP_OK) ret = oled_send_command(line);
        
        // Clear entire line
        for (int i = 0; i < OLED_WIDTH && ret == ESP_OK; i++) {
            ret = oled_send_data(0x00);
        }
    }
    
    // Render text characters
    size_t text_len = strlen(text);
    if (text_len > RENDER_MAX_TEXT_LEN) {
        text_len = RENDER_MAX_TEXT_LEN;
    }
    
    for (size_t i = 0; i < text_len && ret == ESP_OK; i++) {
        ret = draw_char(i * 6, line, text[i]);
        
        // Yield CPU every few characters to avoid watchdog
        if (i % 5 == 0) {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
    
    return ret;
}

/**
 * @brief Render status display
 * 
 * @param status Status data to display
 * @return ESP_OK on success
 */
static esp_err_t render_status_impl(const render_status_t* status) {
    char line_buf[RENDER_MAX_TEXT_LEN + 1];
    esp_err_t ret = ESP_OK;
    
    // Line 0: Title
    ret = render_text_line_impl(0, "LucidConsole v1.0", true);
    
    // Line 1: UART stats
    if (ret == ESP_OK) {
        snprintf(line_buf, sizeof(line_buf), "RX:%u TX:%u", 
                 status->rx_count, status->tx_count);
        ret = render_text_line_impl(1, line_buf, true);
    }
    
    // Line 2: System stats
    if (ret == ESP_OK) {
        snprintf(line_buf, sizeof(line_buf), "Up:%us Heap:%uK", 
                 status->uptime_sec, status->free_heap / 1024);
        ret = render_text_line_impl(2, line_buf, true);
    }
    
    // Line 3: WiFi status
    if (ret == ESP_OK) {
        if (status->wifi_rssi != 0) {
            snprintf(line_buf, sizeof(line_buf), "WiFi: %ddBm", status->wifi_rssi);
        } else {
            snprintf(line_buf, sizeof(line_buf), "WiFi: Disconnected");
        }
        ret = render_text_line_impl(3, line_buf, true);
    }
    
    return ret;
}

/**
 * @brief UI task - processes render queue
 */
static void ui_task(void* pvParameters) {
    render_cmd_t cmd;
    TickType_t last_wake_time = xTaskGetTickCount();
    
    ESP_LOGI(TAG, "UI task started - processing render queue");
    
    while (true) {
        // Wait for render command or timeout
        if (xQueueReceive(render_queue, &cmd, pdMS_TO_TICKS(UI_REFRESH_RATE_MS)) == pdTRUE) {
            
            // Process render command
            esp_err_t ret = ESP_OK;
            switch (cmd.type) {
                case RENDER_CMD_TEXT_LINE:
                    ret = render_text_line_impl(cmd.data.text_line.line, 
                                              cmd.data.text_line.text,
                                              cmd.data.text_line.clear_line);
                    break;
                    
                case RENDER_CMD_CLEAR_SCREEN:
                    ret = oled_clear_screen();
                    break;
                    
                case RENDER_CMD_DISPLAY_POWER:
                    ret = oled_display_power(cmd.data.display_on);
                    break;
                    
                case RENDER_CMD_STATUS_UPDATE:
                    ret = render_status_impl(&cmd.data.status);
                    break;
                    
                default:
                    ESP_LOGW(TAG, "Unknown render command type: %d", cmd.type);
                    break;
            }
            
            if (ret != ESP_OK) {
                ESP_LOGW(TAG, "Render command failed: %s", esp_err_to_name(ret));
            }
        }
        
        // Rate limiting - maintain consistent refresh rate
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(UI_REFRESH_RATE_MS));
    }
}

// Public API Implementation

esp_err_t oled_render_init(void) {
    ESP_LOGI(TAG, "Initializing OLED render system...");
    ESP_LOGI(TAG, "Queue size: %d commands", RENDER_QUEUE_SIZE);
    ESP_LOGI(TAG, "Refresh rate: %d ms (%d FPS)", UI_REFRESH_RATE_MS, 1000/UI_REFRESH_RATE_MS);
    
    // Create render queue
    render_queue = xQueueCreate(RENDER_QUEUE_SIZE, sizeof(render_cmd_t));
    if (render_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create render queue");
        return ESP_ERR_NO_MEM;
    }
    
    // Create UI task
    BaseType_t ret = xTaskCreate(ui_task, "ui_task", UI_TASK_STACK_SIZE, 
                                NULL, UI_TASK_PRIORITY, &ui_task_handle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create UI task");
        vQueueDelete(render_queue);
        render_queue = NULL;
        return ESP_ERR_NO_MEM;
    }
    
    ESP_LOGI(TAG, "✅ OLED render system initialized");
    ESP_LOGI(TAG, "UI task created with priority %d, stack %d bytes", 
             UI_TASK_PRIORITY, UI_TASK_STACK_SIZE);
    
    return ESP_OK;
}

esp_err_t oled_render_text_line(uint8_t line, const char* text, bool clear_line) {
    if (render_queue == NULL || text == NULL || line >= RENDER_MAX_LINES) {
        return ESP_ERR_INVALID_ARG;
    }
    
    render_cmd_t cmd = {
        .type = RENDER_CMD_TEXT_LINE,
        .data.text_line = {
            .line = line,
            .clear_line = clear_line
        }
    };
    
    // Copy text with length limit
    strncpy(cmd.data.text_line.text, text, RENDER_MAX_TEXT_LEN);
    cmd.data.text_line.text[RENDER_MAX_TEXT_LEN] = '\0';
    
    if (xQueueSend(render_queue, &cmd, 0) != pdTRUE) {
        dropped_commands++;
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t oled_render_clear_screen(void) {
    if (render_queue == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    
    render_cmd_t cmd = {
        .type = RENDER_CMD_CLEAR_SCREEN
    };
    
    if (xQueueSend(render_queue, &cmd, 0) != pdTRUE) {
        dropped_commands++;
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t oled_render_display_power(bool on) {
    if (render_queue == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    
    render_cmd_t cmd = {
        .type = RENDER_CMD_DISPLAY_POWER,
        .data.display_on = on
    };
    
    if (xQueueSend(render_queue, &cmd, 0) != pdTRUE) {
        dropped_commands++;
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t oled_render_status_update(const render_status_t* status) {
    if (render_queue == NULL || status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    render_cmd_t cmd = {
        .type = RENDER_CMD_STATUS_UPDATE,
        .data.status = *status
    };
    
    if (xQueueSend(render_queue, &cmd, 0) != pdTRUE) {
        dropped_commands++;
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

void oled_render_get_stats(uint32_t* queued_items, uint32_t* dropped_items) {
    if (queued_items) {
        *queued_items = (render_queue) ? uxQueueMessagesWaiting(render_queue) : 0;
    }
    if (dropped_items) {
        *dropped_items = dropped_commands;
    }
}