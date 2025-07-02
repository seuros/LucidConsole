/*
 * ESP8266 Hardware I2C Bus Driver
 * 
 * Uses the ESP8266 RTOS SDK hardware I2C driver instead of software bit-banging
 * Based on the official ESP8266 SDK I2C example
 */

#pragma once

#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C Configuration - ESP8266 Ideaspark OLED Board
#define I2C_MASTER_NUM          I2C_NUM_0        // I2C port number for master
#define I2C_MASTER_SDA_IO       12               // GPIO12 (D6) - I2C master data
#define I2C_MASTER_SCL_IO       14               // GPIO14 (D5) - I2C master clock
#define I2C_MASTER_FREQ_HZ      100000           // I2C master clock frequency (100kHz)
#define I2C_MASTER_TX_BUF_DISABLE   0            // I2C master doesn't need buffer
#define I2C_MASTER_RX_BUF_DISABLE   0            // I2C master doesn't need buffer
#define I2C_MASTER_TIMEOUT_MS   1000             // Timeout for I2C operations

// I2C Transaction Helpers
#define WRITE_BIT               I2C_MASTER_WRITE
#define READ_BIT                I2C_MASTER_READ
#define ACK_CHECK_EN            0x1              // Enable ACK check from slave
#define ACK_CHECK_DIS           0x0              // Disable ACK check from slave
#define ACK_VAL                 0x0              // I2C ACK value
#define NACK_VAL                0x1              // I2C NACK value

/**
 * @brief Initialize ESP8266 hardware I2C bus with FreeRTOS mutex
 * 
 * Configures GPIO12 and GPIO14 for hardware I2C operation with internal pullups.
 * Creates I2C bus mutex for thread-safe access across multiple FreeRTOS tasks.
 * Uses the ESP8266 SDK I2C driver for reliable communication.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t i2c_hw_bus_init(void);

/**
 * @brief Take I2C bus mutex (FreeRTOS thread-safe)
 * 
 * Must be called before any I2C operations to ensure thread-safe access.
 * 
 * @param timeout_ms Timeout in milliseconds to wait for mutex
 * @return ESP_OK if mutex acquired, ESP_ERR_TIMEOUT if timeout
 */
esp_err_t i2c_hw_bus_lock(uint32_t timeout_ms);

/**
 * @brief Release I2C bus mutex
 * 
 * Must be called after I2C operations to allow other tasks access.
 */
void i2c_hw_bus_unlock(void);

/**
 * @brief Write command byte to I2C device
 * 
 * @param device_addr I2C device address (7-bit)
 * @param reg_addr Register address to write to
 * @param command Command byte to write
 * @return ESP_OK on success, error code on failure
 */
esp_err_t i2c_hw_write_cmd(uint8_t device_addr, uint8_t reg_addr, uint8_t command);

/**
 * @brief Write data buffer to I2C device
 * 
 * @param device_addr I2C device address (7-bit)
 * @param reg_addr Register address to write to
 * @param data Data buffer to write
 * @param data_len Length of data buffer
 * @return ESP_OK on success, error code on failure
 */
esp_err_t i2c_hw_write_data(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, size_t data_len);

/**
 * @brief Read data from I2C device
 * 
 * @param device_addr I2C device address (7-bit)
 * @param reg_addr Register address to read from
 * @param data Buffer to store read data
 * @param data_len Number of bytes to read
 * @return ESP_OK on success, error code on failure
 */
esp_err_t i2c_hw_read_data(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, size_t data_len);

/**
 * @brief Scan I2C bus for devices
 * 
 * Scans all I2C addresses and reports found devices.
 * Useful for debugging I2C connectivity.
 * 
 * @return Number of devices found
 */
int i2c_hw_scan_devices(void);

#ifdef __cplusplus
}
#endif