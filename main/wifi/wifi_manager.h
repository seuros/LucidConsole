/*
 * WiFi Manager - LucidConsole WiFi Configuration System
 * Handles SoftAP provisioning and station mode connection
 */

#pragma once

#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// WiFi Configuration
#define LUCIDUART_AP_SSID_PREFIX    "LUCIDUART_"
#define LUCIDUART_AP_PASSWORD       "luciduart123"
#define LUCIDUART_AP_CHANNEL        1
#define LUCIDUART_AP_MAX_STA_CONN   4

// Custom IP range (10.10.10.x instead of 192.168.4.x)
#define LUCIDUART_AP_IP         "10.10.10.1"
#define LUCIDUART_AP_GATEWAY    "10.10.10.1" 
#define LUCIDUART_AP_NETMASK    "255.255.255.0"

// WiFi connection status (avoid conflicts with esp_wifi_types.h)
typedef enum {
    LUCID_WIFI_STATE_INIT = 0,
    LUCID_WIFI_STATE_AP_MODE,
    LUCID_WIFI_STATE_STA_CONNECTING,
    LUCID_WIFI_STATE_STA_CONNECTED,
    LUCID_WIFI_STATE_STA_DISCONNECTED
} lucid_wifi_state_t;

// WiFi status information for display
typedef struct {
    lucid_wifi_state_t state;
    char ssid[33];          // Current SSID (AP name or connected network)
    char ip_address[16];    // Current IP address
    int8_t rssi;           // Signal strength (STA mode only)
    uint8_t sta_count;     // Connected clients (AP mode only)
    bool provisioned;      // True if STA credentials are stored
} lucid_wifi_status_t;

/**
 * @brief Initialize WiFi manager system
 * 
 * Sets up WiFi driver, event loop, and starts in AP mode for initial provisioning.
 * Creates LUCIDUART_{MAC4} SSID with custom 10.10.10.x IP range.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief Get current WiFi status
 * 
 * Returns current connection state, IP address, SSID, and other status info
 * for display on OLED screen.
 * 
 * @param status Pointer to lucid_wifi_status_t structure to fill
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if status is NULL
 */
esp_err_t wifi_manager_get_status(lucid_wifi_status_t* status);

/**
 * @brief Start SoftAP mode
 * 
 * Creates WiFi access point with LUCIDUART_{MAC4} name and 10.10.10.1 IP.
 * Used for initial device configuration and fallback mode.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t wifi_manager_start_ap(void);

/**
 * @brief Connect to WiFi network (STA mode)
 * 
 * Attempts to connect to stored WiFi credentials or provided SSID/password.
 * 
 * @param ssid WiFi network name (NULL to use stored credentials)
 * @param password WiFi password (NULL to use stored credentials)
 * @return ESP_OK on success, error code on failure
 */
esp_err_t wifi_manager_connect_sta(const char* ssid, const char* password);

/**
 * @brief Save WiFi credentials to NVS
 * 
 * Stores SSID and password for automatic connection on boot.
 * 
 * @param ssid WiFi network name
 * @param password WiFi password
 * @return ESP_OK on success, error code on failure
 */
esp_err_t wifi_manager_save_credentials(const char* ssid, const char* password);

/**
 * @brief Check if WiFi credentials are stored
 * 
 * @return true if credentials are available in NVS, false otherwise
 */
bool wifi_manager_has_credentials(void);

/**
 * @brief Reset to AP mode
 * 
 * Disconnects from STA and starts AP mode for reconfiguration.
 * Used for factory reset or manual reconfiguration.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t wifi_manager_reset_to_ap(void);

/**
 * @brief Get device MAC address formatted for SSID
 * 
 * Returns last 4 MAC address digits for LUCIDUART_{MAC4} SSID.
 * 
 * @param mac4_str Buffer to store 4-character MAC string (minimum 5 bytes)
 * @return ESP_OK on success, error code on failure
 */
esp_err_t wifi_manager_get_mac4(char* mac4_str);

#ifdef __cplusplus
}
#endif