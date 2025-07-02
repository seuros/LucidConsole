/*
 * LucidConsole Configuration
 * 
 * Compile-time feature toggles for modular builds
 */

#pragma once

// ========================================
// DISPLAY MODULE CONFIGURATION
// ========================================

/**
 * Enable OLED Display Support
 * 
 * Set to 1 to include I2C and OLED init/render subsystems.
 * Set to 0 for screenless builds (saves ~10-15KB flash/RAM).
 * 
 * When disabled:
 * - No I2C hardware initialization
 * - No OLED display tasks or queues  
 * - Status output via UART only
 * - Faster boot, lower memory usage
 */
#define CONFIG_ENABLE_OLED_DISPLAY 1

// ========================================  
// I2C HARDWARE CONFIGURATION
// ========================================

#if CONFIG_ENABLE_OLED_DISPLAY
/**
 * I2C Pin Configuration (ESP8266 Ideaspark Board)
 * 
 * Non-standard pins due to board layout constraints.
 * Standard ESP8266 pins (GPIO4/5) are not available.
 */
#define CONFIG_I2C_SDA_PIN    12    // GPIO12 (D6 on NodeMCU)
#define CONFIG_I2C_SCL_PIN    14    // GPIO14 (D5 on NodeMCU) 
#define CONFIG_I2C_FREQ_HZ    100000   // 100kHz I2C frequency

/**
 * OLED Display Configuration
 */
#define CONFIG_OLED_WIDTH     128   // Display width in pixels
#define CONFIG_OLED_HEIGHT    64    // Display height in pixels  
#define CONFIG_OLED_I2C_ADDR  0x3C  // I2C address
#define CONFIG_OLED_POWER_PIN 16    // GPIO16 power control
#endif

// ========================================
// FREERTOS TASK CONFIGURATION  
// ========================================

#if CONFIG_ENABLE_OLED_DISPLAY
/**
 * Display Task Parameters (Gemini Optimized)
 */
#define CONFIG_DISPLAY_TASK_PRIORITY    (tskIDLE_PRIORITY + 2)  // Medium priority
#define CONFIG_DISPLAY_TASK_STACK_SIZE  2048    // Stack size in words (8KB)
#define CONFIG_DISPLAY_UPDATE_RATE_MS   1000    // Update every 1 second
#endif

// ========================================
// UART BRIDGE CONFIGURATION
// ========================================

/**
 * Core UART Bridge Settings (Always Enabled)
 * 
 * These settings are for the main WiFi-to-UART bridge functionality.
 */
#define CONFIG_UART_BAUD_RATE    115200
#define CONFIG_UART_TX_PIN       1      // GPIO1 (TX)
#define CONFIG_UART_RX_PIN       3      // GPIO3 (RX)
#define CONFIG_UART_BUFFER_SIZE  1024   // UART buffer size

// ========================================
// WIFI CONFIGURATION
// ========================================

/**
 * WiFi Access Point Mode (Always Enabled)
 */
#define CONFIG_WIFI_AP_SSID_PREFIX   "LUCIDUART"    // Will become LUCIDUART_XXXX
#define CONFIG_WIFI_AP_PASSWORD      ""             // Open network  
#define CONFIG_WIFI_AP_CHANNEL       1              // WiFi channel
#define CONFIG_WIFI_AP_MAX_CONN      4              // Max connections

/**
 * DHCP Server Configuration
 */
#define CONFIG_DHCP_IP_BASE     "10.10.10"         // IP range: 10.10.10.1-254
#define CONFIG_DHCP_GATEWAY     "10.10.10.1"       // Gateway IP
#define CONFIG_DHCP_NETMASK     "255.255.255.0"    // Subnet mask

// ========================================
// HTTP SERVER CONFIGURATION
// ========================================

/**
 * Web Dashboard Settings
 */
#define CONFIG_HTTP_SERVER_PORT     80              // HTTP port
#define CONFIG_HTTP_MAX_URI_LEN     512             // Max URI length
#define CONFIG_HTTP_MAX_RESP_LEN    2048            // Max response length

// ========================================
// DEBUG AND LOGGING
// ========================================

/**
 * Debug Output Configuration
 */
#define CONFIG_LOG_LEVEL            ESP_LOG_INFO    // Default log level
#define CONFIG_ENABLE_STACK_MONITOR 1               // Stack usage monitoring
#define CONFIG_ENABLE_HEAP_MONITOR  1               // Heap usage monitoring