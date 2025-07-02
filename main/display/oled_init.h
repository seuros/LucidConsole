/*
 * OLED Initialization Module - SSD1306 Display Controller
 * 
 * Handles SSD1306 OLED display initialization with minimal power consumption.
 * Uses shared I2C bus with proper mutex protection.
 * 
 * Display: 128x64 pixel SSD1306 OLED at I2C address 0x3C
 * Power: Controlled by GPIO16 via gpio_init module
 */

#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// SSD1306 Configuration
#define OLED_I2C_ADDR       0x3C    // 7-bit I2C address
#define OLED_WIDTH          128     // Pixels
#define OLED_HEIGHT         64      // Pixels
#define OLED_PAGES          8       // Height / 8

// SSD1306 Command Modes
#define OLED_CMD_MODE       0x00    // Command mode prefix
#define OLED_DATA_MODE      0x40    // Data mode prefix

/**
 * @brief Initialize OLED display
 * 
 * Performs complete SSD1306 initialization sequence:
 * 1. Turn on OLED power via GPIO16
 * 2. Probe I2C device at 0x3C
 * 3. Send minimal SSD1306 initialization commands
 * 4. Clear display and turn on
 * 
 * Requires I2C bus to be already initialized.
 * Uses mutex-protected I2C transactions.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t oled_init(void);

/**
 * @brief Turn OLED display on/off
 * 
 * @param on true to turn display on, false to turn off
 * @return ESP_OK on success
 */
esp_err_t oled_display_power(bool on);

/**
 * @brief Clear entire OLED display
 * 
 * Sets all pixels to black (0).
 * 
 * @return ESP_OK on success
 */
esp_err_t oled_clear_screen(void);

/**
 * @brief Send command to SSD1306
 * 
 * I2C bus must be locked before calling this function.
 * 
 * @param cmd Command byte to send
 * @return ESP_OK on success
 */
esp_err_t oled_send_command(uint8_t cmd);

/**
 * @brief Send data to SSD1306
 * 
 * I2C bus must be locked before calling this function.
 * 
 * @param data Data byte to send
 * @return ESP_OK on success  
 */
esp_err_t oled_send_data(uint8_t data);

/**
 * @brief Test OLED hardware
 * 
 * Performs basic hardware test:
 * 1. Turn on power
 * 2. Probe I2C address
 * 3. Test command sending
 * 
 * @return ESP_OK if hardware responds, error code if failed
 */
esp_err_t oled_test_hardware(void);

#ifdef __cplusplus
}
#endif