/*
 * Web Server - LucidConsole HTTP Dashboard
 * Provides web interface for WiFi configuration and UART bridge control
 */

#pragma once

#include "esp_err.h"
#include "esp_http_server.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// HTTP server configuration
#define LUCIDUART_HTTP_PORT         80
#define LUCIDUART_MAX_URI_LEN       128
#define LUCIDUART_MAX_REQ_HDR_LEN   512

// API endpoints
#define LUCIDUART_API_STATUS        "/api/status"
#define LUCIDUART_API_WIFI_SCAN     "/api/wifi/scan"
#define LUCIDUART_API_WIFI_CONNECT  "/api/wifi/connect"
#define LUCIDUART_API_WIFI_RESET    "/api/wifi/reset"
#define LUCIDUART_API_SYSTEM_INFO   "/api/system/info"
#define LUCIDUART_API_UART_STATS    "/api/uart/stats"

// System status for API responses
typedef struct {
    // System info
    uint32_t uptime_sec;
    uint32_t free_heap;
    const char* firmware_version;
    const char* chip_model;
    
    // WiFi status
    const char* wifi_mode;      // "AP" or "STA"
    const char* ssid;
    const char* ip_address;
    int8_t rssi;               // Signal strength (STA mode)
    uint8_t client_count;      // Connected clients (AP mode)
    
    // UART bridge status
    uint32_t uart_rx_count;
    uint32_t uart_tx_count;
    uint32_t uart_baud_rate;
    bool uart_bridge_active;
} web_system_status_t;

/**
 * @brief Initialize HTTP web server
 * 
 * Starts HTTP server on port 80 with dashboard endpoints.
 * Serves web interface for WiFi configuration and system monitoring.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t web_server_init(void);

/**
 * @brief Stop HTTP web server
 * 
 * Stops the HTTP server and frees resources.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t web_server_stop(void);

/**
 * @brief Check if web server is running
 * 
 * @return true if server is active, false otherwise
 */
bool web_server_is_running(void);

/**
 * @brief Get current system status for API
 * 
 * Collects system, WiFi, and UART status for JSON API responses.
 * 
 * @param status Pointer to status structure to fill
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if status is NULL
 */
esp_err_t web_server_get_system_status(web_system_status_t* status);

/**
 * @brief Set UART bridge statistics callback
 * 
 * Registers callback to get real-time UART RX/TX counters.
 * 
 * @param rx_count_callback Function to get RX byte count
 * @param tx_count_callback Function to get TX byte count
 * @return ESP_OK on success
 */
esp_err_t web_server_set_uart_callbacks(uint32_t (*rx_count_callback)(void),
                                        uint32_t (*tx_count_callback)(void));

/**
 * @brief Broadcast UART data to SSE clients
 * 
 * Sends received UART data to all connected SSE clients for real-time streaming.
 * Data is Base64 encoded for safe JSON transmission.
 * 
 * @param data UART data buffer
 * @param len Length of data
 */
void web_server_broadcast_uart_data(const uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif