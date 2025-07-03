/*
 * Web Server - LucidConsole HTTP Dashboard Implementation
 * Serves web interface and REST API endpoints
 */

#include "web_server.h"
#include "../wifi/wifi_manager.h"
#include "../uart/uart_bridge.h"
#include "esp_log.h"
#include "esp_system.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "mbedtls/base64.h"
#include <string.h>
#include <stdlib.h>

// ESP8266 compatibility defines
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

static const char* TAG = "WEB_SERVER";

// HTTP server handle
static httpd_handle_t server = NULL;

// UART callback functions
static uint32_t (*uart_rx_callback)(void) = NULL;
static uint32_t (*uart_tx_callback)(void) = NULL;

// SSE client management
#define MAX_SSE_CLIENTS 4
static httpd_req_t* sse_clients[MAX_SSE_CLIENTS] = {0};
static SemaphoreHandle_t sse_mutex = NULL;

// HTML Dashboard Content
static const char* html_dashboard = 
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='UTF-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
"<title>LucidConsole Dashboard</title>"
"<style>"
"body { font-family: 'Courier New', monospace; background: #0a0a0a; color: #00ff00; margin: 0; padding: 20px; }"
".container { max-width: 800px; margin: 0 auto; background: #1a1a1a; padding: 20px; border: 2px solid #00ff00; border-radius: 8px; }"
".header { text-align: center; margin-bottom: 30px; }"
".title { font-size: 2em; color: #00ffff; text-shadow: 0 0 10px #00ffff; margin-bottom: 10px; }"
".subtitle { color: #ffff00; }"
".section { margin: 20px 0; padding: 15px; border: 1px solid #333; background: #111; }"
".section-title { color: #00ffff; font-size: 1.2em; margin-bottom: 10px; border-bottom: 1px solid #333; padding-bottom: 5px; }"
".status-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; }"
".status-item { background: #222; padding: 10px; border-left: 3px solid #00ff00; }"
".status-label { color: #888; font-size: 0.9em; }"
".status-value { color: #00ff00; font-weight: bold; font-size: 1.1em; }"
".button { background: #003300; color: #00ff00; border: 1px solid #00ff00; padding: 8px 16px; cursor: pointer; margin: 5px; }"
".button:hover { background: #004400; }"
".wifi-form { background: #222; padding: 15px; margin: 10px 0; }"
".form-group { margin: 10px 0; }"
".form-label { display: block; color: #888; margin-bottom: 5px; }"
".form-input { width: 100%; padding: 8px; background: #333; color: #00ff00; border: 1px solid #555; }"
"#refresh-btn { position: fixed; top: 20px; right: 20px; }"
"</style>"
"</head>"
"<body>"
"<div class='container'>"
"<div class='header'>"
"<div class='title'>🛰️ LucidConsole</div>"
"<div class='subtitle'>ESP8266 WiFi-to-UART Bridge</div>"
"</div>"

"<div class='section'>"
"<div class='section-title'>📊 System Status</div>"
"<div class='status-grid' id='system-status'>"
"<div class='status-item'><div class='status-label'>Uptime</div><div class='status-value' id='uptime'>Loading...</div></div>"
"<div class='status-item'><div class='status-label'>Free Memory</div><div class='status-value' id='memory'>Loading...</div></div>"
"<div class='status-item'><div class='status-label'>Firmware</div><div class='status-value' id='firmware'>v1.0</div></div>"
"<div class='status-item'><div class='status-label'>Chip Model</div><div class='status-value' id='chip'>ESP8266</div></div>"
"</div>"
"</div>"

"<div class='section'>"
"<div class='section-title'>📡 WiFi Status</div>"
"<div class='status-grid' id='wifi-status'>"
"<div class='status-item'><div class='status-label'>Mode</div><div class='status-value' id='wifi-mode'>Loading...</div></div>"
"<div class='status-item'><div class='status-label'>SSID</div><div class='status-value' id='wifi-ssid'>Loading...</div></div>"
"<div class='status-item'><div class='status-label'>IP Address</div><div class='status-value' id='ip-address'>Loading...</div></div>"
"<div class='status-item'><div class='status-label'>Signal/Clients</div><div class='status-value' id='wifi-signal'>Loading...</div></div>"
"</div>"
"</div>"

"<div class='section'>"
"<div class='section-title'>🔧 WiFi Configuration</div>"
"<div class='wifi-form'>"
"<div class='form-group'>"
"<label class='form-label' for='ssid-input'>Network SSID:</label>"
"<input type='text' id='ssid-input' class='form-input' placeholder='Enter WiFi network name'>"
"</div>"
"<div class='form-group'>"
"<label class='form-label' for='password-input'>Password:</label>"
"<input type='password' id='password-input' class='form-input' placeholder='Enter WiFi password'>"
"</div>"
"<button class='button' onclick='connectWiFi()'>Connect to Network</button>"
"<button class='button' onclick='resetToAP()'>Reset to AP Mode</button>"
"</div>"
"</div>"

"<div class='section'>"
"<div class='section-title'>🔌 UART Bridge Status</div>"
"<div class='status-grid' id='uart-status'>"
"<div class='status-item'><div class='status-label'>RX Bytes</div><div class='status-value' id='uart-rx'>Loading...</div></div>"
"<div class='status-item'><div class='status-label'>TX Bytes</div><div class='status-value' id='uart-tx'>Loading...</div></div>"
"<div class='status-item'><div class='status-label'>Baud Rate</div><div class='status-value' id='uart-baud'>115200</div></div>"
"<div class='status-item'><div class='status-label'>Bridge Status</div><div class='status-value' id='uart-active'>Active</div></div>"
"</div>"
"</div>"

"<div class='section'>"
"<div class='section-title'>💻 Serial Terminal</div>"
"<div id='terminal' style='background:#000;color:#0f0;font-family:monospace;padding:10px;height:200px;overflow-y:auto;border:1px solid #0f0;margin-bottom:10px;white-space:pre-wrap;word-wrap:break-word;'></div>"
"<div style='display:flex;'>"
"<input type='text' id='uart-input' style='flex:1;padding:8px;background:#333;color:#0f0;border:1px solid #555;font-family:monospace;' placeholder='Enter command and press Enter' onkeypress='if(event.key===\"Enter\")sendUartCommand()'>"
"<button class='button' onclick='sendUartCommand()'>Send</button>"
"<button class='button' onclick='clearTerminal()'>Clear</button>"
"</div>"
"</div>"

"</div>"

"<button id='refresh-btn' class='button' onclick='refreshStatus()'>🔄 Refresh</button>"

"<script>"
"function updateStatus() {"
"  fetch('/api/status')"
"    .then(response => response.json())"
"    .then(data => {"
"      document.getElementById('uptime').textContent = data.uptime_sec + 's';"
"      document.getElementById('memory').textContent = Math.round(data.free_heap/1024) + 'KB';"
"      document.getElementById('wifi-mode').textContent = data.wifi_mode;"
"      document.getElementById('wifi-ssid').textContent = data.ssid;"
"      document.getElementById('ip-address').textContent = data.ip_address;"
"      document.getElementById('uart-rx').textContent = data.uart_rx_count;"
"      document.getElementById('uart-tx').textContent = data.uart_tx_count;"
"      if(data.wifi_mode === 'AP') {"
"        document.getElementById('wifi-signal').textContent = data.client_count + ' clients';"
"      } else {"
"        document.getElementById('wifi-signal').textContent = data.rssi + 'dBm';"
"      }"
"    })"
"    .catch(err => console.log('Status update failed:', err));"
"}"

"function connectWiFi() {"
"  const ssid = document.getElementById('ssid-input').value;"
"  const password = document.getElementById('password-input').value;"
"  if(!ssid) { alert('Please enter SSID'); return; }"
"  fetch('/api/wifi/connect', {"
"    method: 'POST',"
"    headers: { 'Content-Type': 'application/json' },"
"    body: JSON.stringify({ ssid: ssid, password: password })"
"  })"
"  .then(response => response.json())"
"  .then(data => {"
"    alert(data.message || 'Connection initiated');"
"    setTimeout(updateStatus, 2000);"
"  })"
"  .catch(err => alert('Connection failed: ' + err));"
"}"

"function resetToAP() {"
"  if(confirm('Reset WiFi to AP mode?')) {"
"    fetch('/api/wifi/reset', { method: 'POST' })"
"    .then(response => response.json())"
"    .then(data => {"
"      alert(data.message || 'Reset to AP mode');"
"      setTimeout(updateStatus, 2000);"
"    })"
"    .catch(err => alert('Reset failed: ' + err));"
"  }"
"}"

"function refreshStatus() {"
"  updateStatus();"
"}"

"// Terminal functions"
"let eventSource = null;"

"function initTerminal() {"
"  if(eventSource) eventSource.close();"
"  eventSource = new EventSource('/api/uart/stream');"
"  "
"  eventSource.onmessage = function(event) {"
"    try {"
"      const data = JSON.parse(event.data);"
"      if(data.uart_b64) {"
"        // Decode Base64 UART data"
"        const decoded = atob(data.uart_b64);"
"        appendToTerminal(decoded, 'rx');"
"      }"
"      if(data.connected) {"
"        appendToTerminal('[Connected to UART stream]\\n', 'system');"
"      }"
"    } catch(e) {"
"      console.error('SSE parse error:', e);"
"    }"
"  };"
"  "
"  eventSource.onerror = function(err) {"
"    console.error('SSE error:', err);"
"    appendToTerminal('[Stream disconnected]\\n', 'system');"
"    setTimeout(initTerminal, 5000);  // Reconnect after 5s"
"  };"
"}"

"function appendToTerminal(text, type) {"
"  const terminal = document.getElementById('terminal');"
"  const timestamp = new Date().toLocaleTimeString();"
"  let prefix = '';"
"  "
"  if(type === 'tx') {"
"    prefix = '> ';"
"  } else if(type === 'rx') {"
"    prefix = '';"
"  } else if(type === 'system') {"
"    prefix = '* ';"
"  }"
"  "
"  terminal.textContent += prefix + text;"
"  terminal.scrollTop = terminal.scrollHeight;"
"}"

"function sendUartCommand() {"
"  const input = document.getElementById('uart-input');"
"  const cmd = input.value;"
"  if(!cmd) return;"
"  "
"  // Add newline if not present"
"  const dataToSend = cmd.endsWith('\\n') ? cmd : cmd + '\\n';"
"  "
"  fetch('/api/uart/send', {"
"    method: 'POST',"
"    headers: { 'Content-Type': 'application/json' },"
"    body: JSON.stringify({ data: dataToSend })"
"  })"
"  .then(response => response.json())"
"  .then(data => {"
"    if(data.status === 'sent') {"
"      appendToTerminal(dataToSend, 'tx');"
"      input.value = '';"
"    } else {"
"      appendToTerminal('[Send failed: ' + (data.error || 'Unknown error') + ']\\n', 'system');"
"    }"
"  })"
"  .catch(err => {"
"    console.error('Send error:', err);"
"    appendToTerminal('[Send error: ' + err + ']\\n', 'system');"
"  });"
"}"

"function clearTerminal() {"
"  document.getElementById('terminal').textContent = '';"
"  appendToTerminal('[Terminal cleared]\\n', 'system');"
"}"

"// Auto-refresh status every 5 seconds"
"setInterval(updateStatus, 5000);"
"updateStatus();"

"// Initialize terminal on load"
"window.addEventListener('load', function() {"
"  initTerminal();"
"  appendToTerminal('[LucidConsole Serial Terminal]\\n', 'system');"
"  appendToTerminal('[Type commands and press Enter to send]\\n\\n', 'system');"
"});"
"</script>"
"</body>"
"</html>";

/**
 * @brief Root handler - serves dashboard HTML
 */
static esp_err_t dashboard_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_dashboard, strlen(html_dashboard));
    return ESP_OK;
}

/**
 * @brief API status endpoint - returns system status as JSON
 */
static esp_err_t api_status_handler(httpd_req_t *req) {
    web_system_status_t status;
    esp_err_t ret = web_server_get_system_status(&status);
    if (ret != ESP_OK) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    cJSON *json = cJSON_CreateObject();
    cJSON *uptime = cJSON_CreateNumber(status.uptime_sec);
    cJSON *free_heap = cJSON_CreateNumber(status.free_heap);
    cJSON *firmware = cJSON_CreateString(status.firmware_version);
    cJSON *chip = cJSON_CreateString(status.chip_model);
    cJSON *wifi_mode = cJSON_CreateString(status.wifi_mode);
    cJSON *ssid = cJSON_CreateString(status.ssid);
    cJSON *ip = cJSON_CreateString(status.ip_address);
    cJSON *rssi = cJSON_CreateNumber(status.rssi);
    cJSON *clients = cJSON_CreateNumber(status.client_count);
    cJSON *uart_rx = cJSON_CreateNumber(status.uart_rx_count);
    cJSON *uart_tx = cJSON_CreateNumber(status.uart_tx_count);
    cJSON *uart_baud = cJSON_CreateNumber(status.uart_baud_rate);
    cJSON *uart_active = cJSON_CreateBool(status.uart_bridge_active);
    
    cJSON_AddItemToObject(json, "uptime_sec", uptime);
    cJSON_AddItemToObject(json, "free_heap", free_heap);
    cJSON_AddItemToObject(json, "firmware_version", firmware);
    cJSON_AddItemToObject(json, "chip_model", chip);
    cJSON_AddItemToObject(json, "wifi_mode", wifi_mode);
    cJSON_AddItemToObject(json, "ssid", ssid);
    cJSON_AddItemToObject(json, "ip_address", ip);
    cJSON_AddItemToObject(json, "rssi", rssi);
    cJSON_AddItemToObject(json, "client_count", clients);
    cJSON_AddItemToObject(json, "uart_rx_count", uart_rx);
    cJSON_AddItemToObject(json, "uart_tx_count", uart_tx);
    cJSON_AddItemToObject(json, "uart_baud_rate", uart_baud);
    cJSON_AddItemToObject(json, "uart_bridge_active", uart_active);
    
    const char *json_string = cJSON_Print(json);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_string, strlen(json_string));
    
    free((void *)json_string);
    cJSON_Delete(json);
    return ESP_OK;
}

/**
 * @brief API WiFi connect endpoint - connects to specified network
 */
static esp_err_t api_wifi_connect_handler(httpd_req_t *req) {
    char content[200];
    size_t recv_size = MIN(req->content_len, sizeof(content) - 1);
    
    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[ret] = '\0';
    
    cJSON *json = cJSON_Parse(content);
    if (!json) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    cJSON *ssid_json = cJSON_GetObjectItem(json, "ssid");
    cJSON *password_json = cJSON_GetObjectItem(json, "password");
    
    if (!cJSON_IsString(ssid_json)) {
        cJSON_Delete(json);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    const char *ssid = cJSON_GetStringValue(ssid_json);
    const char *password = cJSON_IsString(password_json) ? cJSON_GetStringValue(password_json) : "";
    
    ESP_LOGI(TAG, "WiFi connect request: %s", ssid);
    
    // Save credentials and attempt connection
    wifi_manager_save_credentials(ssid, password);
    esp_err_t connect_result = wifi_manager_connect_sta(ssid, password);
    
    cJSON *response = cJSON_CreateObject();
    if (connect_result == ESP_OK) {
        cJSON_AddStringToObject(response, "status", "success");
        cJSON_AddStringToObject(response, "message", "Connection initiated");
    } else {
        cJSON_AddStringToObject(response, "status", "error");
        cJSON_AddStringToObject(response, "message", "Connection failed");
    }
    
    const char *response_string = cJSON_Print(response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_string, strlen(response_string));
    
    free((void *)response_string);
    cJSON_Delete(response);
    cJSON_Delete(json);
    return ESP_OK;
}

/**
 * @brief API WiFi reset endpoint - resets to AP mode
 */
static esp_err_t api_wifi_reset_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "WiFi reset to AP mode requested");
    
    esp_err_t reset_result = wifi_manager_reset_to_ap();
    
    cJSON *response = cJSON_CreateObject();
    if (reset_result == ESP_OK) {
        cJSON_AddStringToObject(response, "status", "success");
        cJSON_AddStringToObject(response, "message", "Reset to AP mode");
    } else {
        cJSON_AddStringToObject(response, "status", "error");
        cJSON_AddStringToObject(response, "message", "Reset failed");
    }
    
    const char *response_string = cJSON_Print(response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_string, strlen(response_string));
    
    free((void *)response_string);
    cJSON_Delete(response);
    return ESP_OK;
}

/**
 * @brief API UART send endpoint - sends data to UART
 */
static esp_err_t api_uart_send_handler(httpd_req_t *req) {
    char content[256];
    size_t recv_size = MIN(req->content_len, sizeof(content) - 1);
    
    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        } else {
            httpd_resp_send_500(req);
        }
        return ESP_FAIL;
    }
    content[ret] = '\0';
    
    // Parse JSON {"data": "command"}
    cJSON *json = cJSON_Parse(content);
    if (!json) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "{\"error\":\"Invalid JSON\"}", -1);
        return ESP_FAIL;
    }
    
    cJSON *data_item = cJSON_GetObjectItem(json, "data");
    if (!cJSON_IsString(data_item) || !data_item->valuestring) {
        cJSON_Delete(json);
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "{\"error\":\"Missing data field\"}", -1);
        return ESP_FAIL;
    }
    
    // Send to UART
    const char* uart_data = data_item->valuestring;
    int bytes_sent = uart_bridge_send((const uint8_t*)uart_data, strlen(uart_data));
    
    ESP_LOGI(TAG, "UART TX via API: %s (%d bytes)", uart_data, bytes_sent);
    
    // Create response
    cJSON *response = cJSON_CreateObject();
    if (bytes_sent > 0) {
        cJSON_AddStringToObject(response, "status", "sent");
        cJSON_AddNumberToObject(response, "bytes", bytes_sent);
    } else {
        cJSON_AddStringToObject(response, "status", "failed");
        cJSON_AddStringToObject(response, "error", "UART send failed");
    }
    
    const char *response_string = cJSON_Print(response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_string, strlen(response_string));
    
    free((void *)response_string);
    cJSON_Delete(response);
    cJSON_Delete(json);
    
    return ESP_OK;
}

/**
 * @brief SSE endpoint for real-time UART data streaming
 */
static esp_err_t api_uart_stream_handler(httpd_req_t *req) {
    // Set SSE headers
    httpd_resp_set_type(req, "text/event-stream");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    httpd_resp_set_hdr(req, "Connection", "keep-alive");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    // Send headers immediately
    httpd_resp_send_chunk(req, NULL, 0);
    
    // Find a free client slot
    int client_id = -1;
    if (sse_mutex) {
        xSemaphoreTake(sse_mutex, portMAX_DELAY);
        for (int i = 0; i < MAX_SSE_CLIENTS; i++) {
            if (sse_clients[i] == NULL) {
                sse_clients[i] = req;
                client_id = i;
                ESP_LOGI(TAG, "SSE client connected (slot %d)", i);
                break;
            }
        }
        xSemaphoreGive(sse_mutex);
    }
    
    if (client_id == -1) {
        ESP_LOGW(TAG, "SSE connection rejected - max clients reached");
        return ESP_FAIL;
    }
    
    // Send initial connection message
    const char* connect_msg = "data: {\"connected\": true}\n\n";
    httpd_resp_send_chunk(req, connect_msg, strlen(connect_msg));
    
    // Keep connection alive
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Check if our slot is still valid
        bool is_connected = false;
        if (sse_mutex) {
            xSemaphoreTake(sse_mutex, portMAX_DELAY);
            if (sse_clients[client_id] == req) {
                is_connected = true;
                // Send heartbeat to keep connection alive
                const char* heartbeat = ": heartbeat\n\n";
                if (httpd_resp_send_chunk(req, heartbeat, strlen(heartbeat)) != ESP_OK) {
                    sse_clients[client_id] = NULL;
                    is_connected = false;
                }
            }
            xSemaphoreGive(sse_mutex);
        }
        
        if (!is_connected) {
            ESP_LOGI(TAG, "SSE client disconnected (slot %d)", client_id);
            break;
        }
    }
    
    return ESP_OK;
}

/**
 * @brief Broadcast UART data to all SSE clients
 * This function is called from UART RX callback
 */
void web_server_broadcast_uart_data(const uint8_t* data, size_t len) {
    if (!sse_mutex || !data || len == 0) {
        return;
    }
    
    // Base64 encode the data for safe JSON transmission
    unsigned char b64_buf[512];
    size_t b64_len = 0;
    
    int ret = mbedtls_base64_encode(b64_buf, sizeof(b64_buf), &b64_len, data, len);
    if (ret != 0) {
        ESP_LOGW(TAG, "Failed to encode UART data to Base64");
        return;
    }
    
    // Create SSE message with Base64 encoded data
    char sse_msg[600];
    snprintf(sse_msg, sizeof(sse_msg), "data: {\"uart_b64\":\"%.*s\",\"len\":%d}\n\n", 
             (int)b64_len, b64_buf, (int)len);
    
    // Send to all connected SSE clients
    xSemaphoreTake(sse_mutex, portMAX_DELAY);
    for (int i = 0; i < MAX_SSE_CLIENTS; i++) {
        if (sse_clients[i] != NULL) {
            if (httpd_resp_send_chunk(sse_clients[i], sse_msg, strlen(sse_msg)) != ESP_OK) {
                ESP_LOGW(TAG, "Failed to send to SSE client %d, disconnecting", i);
                sse_clients[i] = NULL;
            }
        }
    }
    xSemaphoreGive(sse_mutex);
}

esp_err_t web_server_get_system_status(web_system_status_t* status) {
    if (!status) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Get WiFi status
    lucid_wifi_status_t wifi_status;
    wifi_manager_get_status(&wifi_status);
    
    // Fill system status
    status->uptime_sec = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
    status->free_heap = esp_get_free_heap_size();
    status->firmware_version = "1.0.0";
    status->chip_model = "ESP8266EX";
    
    // Map WiFi status
    switch (wifi_status.state) {
        case LUCID_WIFI_STATE_AP_MODE:
            status->wifi_mode = "AP";
            break;
        case LUCID_WIFI_STATE_STA_CONNECTED:
            status->wifi_mode = "STA";
            break;
        case LUCID_WIFI_STATE_STA_CONNECTING:
            status->wifi_mode = "Connecting";
            break;
        default:
            status->wifi_mode = "Unknown";
            break;
    }
    
    status->ssid = wifi_status.ssid;
    status->ip_address = wifi_status.ip_address;
    status->rssi = wifi_status.rssi;
    status->client_count = wifi_status.sta_count;
    
    // UART bridge status (use callbacks if available)
    status->uart_rx_count = uart_rx_callback ? uart_rx_callback() : 0;
    status->uart_tx_count = uart_tx_callback ? uart_tx_callback() : 0;
    status->uart_baud_rate = 115200;  // Default for now
    status->uart_bridge_active = true;  // Bridge is now active
    
    return ESP_OK;
}

esp_err_t web_server_set_uart_callbacks(uint32_t (*rx_count_callback)(void),
                                        uint32_t (*tx_count_callback)(void)) {
    uart_rx_callback = rx_count_callback;
    uart_tx_callback = tx_count_callback;
    return ESP_OK;
}

bool web_server_is_running(void) {
    return server != NULL;
}

esp_err_t web_server_stop(void) {
    if (server) {
        esp_err_t ret = httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "HTTP server stopped");
        return ret;
    }
    return ESP_OK;
}

esp_err_t web_server_init(void) {
    if (server) {
        ESP_LOGW(TAG, "HTTP server already running");
        return ESP_OK;
    }
    
    // Starting HTTP server
    
    // Initialize SSE mutex
    if (!sse_mutex) {
        sse_mutex = xSemaphoreCreateMutex();
        if (!sse_mutex) {
            ESP_LOGE(TAG, "Failed to create SSE mutex");
            return ESP_FAIL;
        }
    }
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = LUCIDUART_HTTP_PORT;
    config.max_uri_handlers = 10;  // Increased for new endpoints
    config.max_open_sockets = 6;   // Increased for SSE connections
    config.stack_size = 8192;
    
    esp_err_t ret = httpd_start(&server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Register URI handlers
    httpd_uri_t dashboard_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = dashboard_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &dashboard_uri);
    
    httpd_uri_t api_status_uri = {
        .uri = LUCIDUART_API_STATUS,
        .method = HTTP_GET,
        .handler = api_status_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &api_status_uri);
    
    httpd_uri_t api_wifi_connect_uri = {
        .uri = LUCIDUART_API_WIFI_CONNECT,
        .method = HTTP_POST,
        .handler = api_wifi_connect_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &api_wifi_connect_uri);
    
    httpd_uri_t api_wifi_reset_uri = {
        .uri = LUCIDUART_API_WIFI_RESET,
        .method = HTTP_POST,
        .handler = api_wifi_reset_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &api_wifi_reset_uri);
    
    httpd_uri_t api_uart_send_uri = {
        .uri = "/api/uart/send",
        .method = HTTP_POST,
        .handler = api_uart_send_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &api_uart_send_uri);
    
    httpd_uri_t api_uart_stream_uri = {
        .uri = "/api/uart/stream",
        .method = HTTP_GET,
        .handler = api_uart_stream_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &api_uart_stream_uri);
    
    ESP_LOGI(TAG, "HTTP server started on port %d", LUCIDUART_HTTP_PORT);
    
    return ESP_OK;
}