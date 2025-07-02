/*
 * WiFi Manager - LucidConsole WiFi Configuration System
 * Implements SoftAP provisioning with LUCIDUART_{MAC4} SSID and 10.10.10.x IP range
 */

#include "wifi_manager.h"
#include "esp_log.h"
#include "tcpip_adapter.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_system.h"
#include <string.h>

static const char* TAG = "WIFI_MGR";

// WiFi manager state
static lucid_wifi_status_t current_status = {0};
static bool wifi_initialized = false;

// NVS keys for WiFi credentials
#define NVS_WIFI_NAMESPACE  "wifi_config"
#define NVS_WIFI_SSID_KEY   "ssid"
#define NVS_WIFI_PASS_KEY   "password"

/**
 * @brief WiFi event handler
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_AP_START:
                ESP_LOGI(TAG, "WiFi AP started");
                current_status.state = LUCID_WIFI_STATE_AP_MODE;
                break;
                
            case WIFI_EVENT_AP_STACONNECTED: {
                wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
                ESP_LOGI(TAG, "Client connected to AP: " MACSTR, MAC2STR(event->mac));
                current_status.sta_count++;
                break;
            }
            
            case WIFI_EVENT_AP_STADISCONNECTED: {
                wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
                ESP_LOGI(TAG, "Client disconnected from AP: " MACSTR, MAC2STR(event->mac));
                if (current_status.sta_count > 0) {
                    current_status.sta_count--;
                }
                break;
            }
            
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi STA started");
                current_status.state = LUCID_WIFI_STATE_STA_CONNECTING;
                esp_wifi_connect();
                break;
                
            case WIFI_EVENT_STA_CONNECTED: {
                wifi_event_sta_connected_t* event = (wifi_event_sta_connected_t*) event_data;
                ESP_LOGI(TAG, "Connected to WiFi network: %s", event->ssid);
                strncpy(current_status.ssid, (char*)event->ssid, sizeof(current_status.ssid) - 1);
                current_status.rssi = 0; // Will be updated by IP event
                break;
            }
            
            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;
                ESP_LOGW(TAG, "Disconnected from WiFi (reason: %d)", event->reason);
                current_status.state = LUCID_WIFI_STATE_STA_DISCONNECTED;
                strcpy(current_status.ip_address, "0.0.0.0");
                
                // Try to reconnect (simple strategy)
                ESP_LOGI(TAG, "Attempting to reconnect...");
                esp_wifi_connect();
                break;
            }
            
            default:
                break;
        }
    } else if (event_base == IP_EVENT) {
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP: {
                ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
                ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
                snprintf(current_status.ip_address, sizeof(current_status.ip_address), 
                        IPSTR, IP2STR(&event->ip_info.ip));
                current_status.state = LUCID_WIFI_STATE_STA_CONNECTED;
                
                // Get RSSI
                wifi_ap_record_t ap_info;
                if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
                    current_status.rssi = ap_info.rssi;
                }
                break;
            }
            
            case IP_EVENT_AP_STAIPASSIGNED: {
                ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
                ESP_LOGI(TAG, "AP assigned IP to client: " IPSTR, IP2STR(&event->ip_info.ip));
                break;
            }
            
            default:
                break;
        }
    }
}

esp_err_t wifi_manager_get_mac4(char* mac4_str) {
    if (!mac4_str) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t mac[6];
    esp_err_t ret = esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Format last 4 MAC digits as hex string
    snprintf(mac4_str, 5, "%02X%02X", mac[4], mac[5]);
    return ESP_OK;
}

esp_err_t wifi_manager_start_ap(void) {
    ESP_LOGI(TAG, "Starting WiFi AP mode...");
    
    // Get MAC for SSID
    char mac4[5];
    esp_err_t ret = wifi_manager_get_mac4(mac4);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get MAC address: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Configure AP
    wifi_config_t wifi_config = {
        .ap = {
            .channel = LUCIDUART_AP_CHANNEL,
            .password = LUCIDUART_AP_PASSWORD,
            .max_connection = LUCIDUART_AP_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    
    // Create LUCIDUART_{MAC4} SSID
    snprintf((char*)wifi_config.ap.ssid, sizeof(wifi_config.ap.ssid),
             "%s%s", LUCIDUART_AP_SSID_PREFIX, mac4);
    wifi_config.ap.ssid_len = strlen((char*)wifi_config.ap.ssid);
    
    ESP_LOGI(TAG, "Creating AP: %s", wifi_config.ap.ssid);
    
    // Set WiFi mode and configuration
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // Store AP info in status
    strncpy(current_status.ssid, (char*)wifi_config.ap.ssid, sizeof(current_status.ssid) - 1);
    strcpy(current_status.ip_address, LUCIDUART_AP_IP);
    current_status.sta_count = 0;
    current_status.state = LUCID_WIFI_STATE_AP_MODE;
    
    ESP_LOGI(TAG, "WiFi AP started: %s @ %s", current_status.ssid, current_status.ip_address);
    return ESP_OK;
}

static esp_err_t configure_ap_netif(void) {
    // Configure custom IP range (10.10.10.x) using ESP8266 tcpip_adapter
    tcpip_adapter_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 10, 10, 10, 1);
    IP4_ADDR(&ip_info.gw, 10, 10, 10, 1);  
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    
    esp_err_t ret = tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
    if (ret != ESP_OK && ret != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED) {
        ESP_LOGE(TAG, "Failed to stop DHCP server: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set IP info: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start DHCP server: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "DHCP server configured for 10.10.10.x range");
    return ESP_OK;
}

bool wifi_manager_has_credentials(void) {
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_WIFI_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        return false;
    }
    
    size_t required_size = 0;
    ret = nvs_get_str(nvs_handle, NVS_WIFI_SSID_KEY, NULL, &required_size);
    nvs_close(nvs_handle);
    
    return (ret == ESP_OK && required_size > 0);
}

esp_err_t wifi_manager_save_credentials(const char* ssid, const char* password) {
    if (!ssid || !password) {
        return ESP_ERR_INVALID_ARG;
    }
    
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_WIFI_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = nvs_set_str(nvs_handle, NVS_WIFI_SSID_KEY, ssid);
    if (ret == ESP_OK) {
        ret = nvs_set_str(nvs_handle, NVS_WIFI_PASS_KEY, password);
    }
    
    if (ret == ESP_OK) {
        ret = nvs_commit(nvs_handle);
        ESP_LOGI(TAG, "WiFi credentials saved: %s", ssid);
        current_status.provisioned = true;
    } else {
        ESP_LOGE(TAG, "Failed to save credentials: %s", esp_err_to_name(ret));
    }
    
    nvs_close(nvs_handle);
    return ret;
}

esp_err_t wifi_manager_connect_sta(const char* ssid, const char* password) {
    wifi_config_t wifi_config = {0};
    
    if (ssid && password) {
        // Use provided credentials
        strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
        strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    } else {
        // Load from NVS
        nvs_handle_t nvs_handle;
        esp_err_t ret = nvs_open(NVS_WIFI_NAMESPACE, NVS_READONLY, &nvs_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "No stored WiFi credentials");
            return ESP_ERR_NOT_FOUND;
        }
        
        size_t ssid_len = sizeof(wifi_config.sta.ssid);
        size_t pass_len = sizeof(wifi_config.sta.password);
        
        ret = nvs_get_str(nvs_handle, NVS_WIFI_SSID_KEY, (char*)wifi_config.sta.ssid, &ssid_len);
        if (ret == ESP_OK) {
            ret = nvs_get_str(nvs_handle, NVS_WIFI_PASS_KEY, (char*)wifi_config.sta.password, &pass_len);
        }
        
        nvs_close(nvs_handle);
        
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to load WiFi credentials: %s", esp_err_to_name(ret));
            return ret;
        }
    }
    
    ESP_LOGI(TAG, "Connecting to WiFi: %s", wifi_config.sta.ssid);
    
    // Set WiFi mode and configuration
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    current_status.state = LUCID_WIFI_STATE_STA_CONNECTING;
    strncpy(current_status.ssid, (char*)wifi_config.sta.ssid, sizeof(current_status.ssid) - 1);
    strcpy(current_status.ip_address, "0.0.0.0");
    
    return ESP_OK;
}

esp_err_t wifi_manager_reset_to_ap(void) {
    ESP_LOGI(TAG, "Resetting to AP mode...");
    
    esp_err_t ret = esp_wifi_stop();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "WiFi stop failed: %s", esp_err_to_name(ret));
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    return wifi_manager_start_ap();
}

esp_err_t wifi_manager_get_status(lucid_wifi_status_t* status) {
    if (!status) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *status = current_status;
    return ESP_OK;
}

esp_err_t wifi_manager_init(void) {
    if (wifi_initialized) {
        ESP_LOGW(TAG, "WiFi manager already initialized");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Initializing WiFi manager...");
    
    // Initialize tcpip adapter for ESP8266
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Initialize WiFi with default config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // Register event handlers (ESP8266 style)
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                                               &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, 
                                               &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED,
                                               &wifi_event_handler, NULL));
    
    // Configure custom IP range for AP
    esp_err_t ret = configure_ap_netif();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure AP network interface");
        return ret;
    }
    
    // Initialize status
    current_status.state = LUCID_WIFI_STATE_INIT;
    current_status.provisioned = wifi_manager_has_credentials();
    strcpy(current_status.ip_address, "0.0.0.0");
    current_status.rssi = -100;
    current_status.sta_count = 0;
    
    wifi_initialized = true;
    
    // Start in AP mode by default, or try STA if credentials exist
    if (current_status.provisioned) {
        ESP_LOGI(TAG, "Found stored WiFi credentials, attempting STA connection...");
        ret = wifi_manager_connect_sta(NULL, NULL);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "STA connection failed, falling back to AP mode");
            wifi_manager_start_ap();
        }
    } else {
        ESP_LOGI(TAG, "No stored WiFi credentials, starting in AP mode");
        wifi_manager_start_ap();
    }
    
    ESP_LOGI(TAG, "WiFi manager initialized");
    return ESP_OK;
}