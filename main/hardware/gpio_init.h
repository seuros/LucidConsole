/*
 * GPIO Initialization Module - ESP8266 Hardware-Specific Setup
 * 
 * Handles ESP8266-specific GPIO initialization including:
 * - GPIO16 RTC domain power control with proper sequencing
 * - HSPI conflict resolution for GPIO12/14 I2C pins
 * - Bootstrap pin safety (GPIO0/2/15)
 * 
 * Must be called early in boot process before any peripheral initialization.
 */

#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// GPIO Pin Definitions - ESP8266 SDK Standard
#define GPIO_OLED_POWER     16    // RTC domain - controls OLED VCC (0=ON, 1=OFF)
#define GPIO_I2C_SDA        14    // I2C data line (ESP8266 SDK standard)
#define GPIO_I2C_SCL        2     // I2C clock line (ESP8266 SDK standard)
#define GPIO_BOOT_BUTTON    0     // Bootstrap pin - handle with care

/**
 * @brief Early GPIO initialization
 * 
 * Performs critical GPIO setup that must happen before any peripheral init:
 * 1. Configure GPIO16 as output and set to OFF (power control)
 * 2. Resolve HSPI conflict by deinitializing SPI controller
 * 3. Configure GPIO12/14 for I2C use with proper timing
 * 
 * Call this FIRST in app_main(), before any other hardware initialization.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gpio_early_init(void);

/**
 * @brief Turn OLED power ON
 * 
 * Sets GPIO16 to proper level for OLED power ON.
 * Includes stabilization delay for power rail.
 * 
 * @return ESP_OK on success
 */
esp_err_t gpio_oled_power_on(void);

/**
 * @brief Turn OLED power OFF
 * 
 * Sets GPIO16 to proper level for OLED power OFF.
 * 
 * @return ESP_OK on success
 */
esp_err_t gpio_oled_power_off(void);

/**
 * @brief Initialize boot button GPIO (after WiFi init)
 * 
 * Configures GPIO0 for button input with interrupt capability.
 * Must be called AFTER WiFi initialization to avoid boot issues.
 * 
 * @return ESP_OK on success
 */
esp_err_t gpio_boot_button_init(void);

/**
 * @brief Get boot button state
 * 
 * @return true if button is pressed, false if released
 */
bool gpio_boot_button_pressed(void);

#ifdef __cplusplus
}
#endif