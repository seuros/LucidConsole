/*
 * UART Bridge - LucidConsole Serial Communication Bridge
 * Handles UART initialization, GPIO control, and data bridging
 */

#pragma once

#include "esp_err.h"
#include "driver/uart.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// UART configuration
#define LUCIDUART_UART_NUM          UART_NUM_0      // Use UART0 (GPIO1/GPIO3)
#define LUCIDUART_DEFAULT_BAUD      115200          // Default baud rate
#define LUCIDUART_TX_PIN            GPIO_NUM_1      // GPIO1 (TX)
#define LUCIDUART_RX_PIN            GPIO_NUM_3      // GPIO3 (RX)
#define LUCIDUART_RTS_PIN           -1                  // No hardware flow control (ESP8266)
#define LUCIDUART_CTS_PIN           -1                  // No hardware flow control (ESP8266)

// Buffer sizes
#define LUCIDUART_TX_BUF_SIZE       1024            // TX buffer size
#define LUCIDUART_RX_BUF_SIZE       1024            // RX buffer size
#define LUCIDUART_QUEUE_SIZE        10              // UART event queue size

// Bridge statistics
typedef struct {
    uint32_t rx_bytes;          // Total bytes received from UART
    uint32_t tx_bytes;          // Total bytes transmitted to UART
    uint32_t rx_errors;         // RX errors (overflow, parity, etc.)
    uint32_t tx_errors;         // TX errors
    uint32_t bridge_uptime;     // Time since bridge started (seconds)
    bool bridge_active;         // Bridge is currently active
    uint32_t current_baud;      // Current baud rate
    uint32_t connected_clients; // Number of WebSocket clients
} uart_bridge_stats_t;

// Bridge configuration
typedef struct {
    uint32_t baud_rate;         // UART baud rate
    uart_word_length_t data_bits;   // Data bits (5-8)
    uart_parity_t parity;       // Parity (none, even, odd)
    uart_stop_bits_t stop_bits; // Stop bits (1, 1.5, 2)
    bool echo_enabled;          // Local echo mode
    bool timestamp_enabled;     // Add timestamps to data
} uart_bridge_config_t;

/**
 * @brief Initialize UART bridge system
 * 
 * Sets up UART driver, GPIO configuration, and internal buffers.
 * Creates UART event handling task for data processing.
 * 
 * @param config Pointer to bridge configuration (NULL for defaults)
 * @return ESP_OK on success, error code on failure
 */
esp_err_t uart_bridge_init(const uart_bridge_config_t* config);

/**
 * @brief Start UART bridge operation
 * 
 * Begins bridging data between UART and network clients.
 * Creates background tasks for data handling.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t uart_bridge_start(void);

/**
 * @brief Stop UART bridge operation
 * 
 * Stops data bridging and background tasks.
 * UART driver remains initialized for reconfiguration.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t uart_bridge_stop(void);

/**
 * @brief Deinitialize UART bridge system
 * 
 * Stops bridge, deinitializes UART driver, and frees resources.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t uart_bridge_deinit(void);

/**
 * @brief Send data to UART
 * 
 * Transmits data from network client to serial device.
 * Thread-safe, can be called from WebSocket handlers.
 * 
 * @param data Data buffer to send
 * @param length Number of bytes to send
 * @return Number of bytes sent, or -1 on error
 */
int uart_bridge_send(const uint8_t* data, size_t length);

/**
 * @brief Register data receive callback
 * 
 * Sets callback function to handle data received from UART.
 * Used by WebSocket server to forward data to clients.
 * 
 * @param callback Function to call when UART data is received
 * @return ESP_OK on success
 */
esp_err_t uart_bridge_set_rx_callback(void (*callback)(const uint8_t* data, size_t length));

/**
 * @brief Get bridge statistics
 * 
 * Returns current bridge statistics for monitoring and display.
 * 
 * @param stats Pointer to stats structure to fill
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if stats is NULL
 */
esp_err_t uart_bridge_get_stats(uart_bridge_stats_t* stats);

/**
 * @brief Reset bridge statistics
 * 
 * Resets RX/TX counters and error counts to zero.
 * 
 * @return ESP_OK on success
 */
esp_err_t uart_bridge_reset_stats(void);

/**
 * @brief Update UART configuration
 * 
 * Changes UART parameters (baud rate, data format) on the fly.
 * Bridge will restart with new configuration.
 * 
 * @param config New configuration parameters
 * @return ESP_OK on success, error code on failure
 */
esp_err_t uart_bridge_update_config(const uart_bridge_config_t* config);

/**
 * @brief Check if bridge is active
 * 
 * @return true if bridge is actively running, false otherwise
 */
bool uart_bridge_is_active(void);

/**
 * @brief Enable/disable GPIO TX/RX control
 * 
 * Controls GPIO pins to enable/disable UART transmission.
 * Useful for preventing line conflicts during configuration.
 * 
 * @param tx_enable Enable TX output (true) or tri-state (false)
 * @param rx_enable Enable RX input (true) or ignore (false)
 * @return ESP_OK on success, error code on failure
 */
esp_err_t uart_bridge_gpio_control(bool tx_enable, bool rx_enable);

/**
 * @brief Get UART RX byte count (for display callback)
 * 
 * Returns total received bytes for OLED display and web dashboard.
 * Thread-safe function pointer for external modules.
 * 
 * @return Total RX byte count
 */
uint32_t uart_bridge_get_rx_count(void);

/**
 * @brief Get UART TX byte count (for display callback)
 * 
 * Returns total transmitted bytes for OLED display and web dashboard.
 * Thread-safe function pointer for external modules.
 * 
 * @return Total TX byte count
 */
uint32_t uart_bridge_get_tx_count(void);

#ifdef __cplusplus
}
#endif