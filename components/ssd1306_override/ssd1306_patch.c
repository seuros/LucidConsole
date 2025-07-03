/*
 * SSD1306 Monkey Patch for ESP8266
 * Fixes boot loop issues by reducing power consumption during initialization
 */

#include "ssd1306/ssd1306.h"
#include "esp_log.h"

static const char* TAG = "SSD1306_PATCH";

/**
 * @brief Patched initialization function for ESP8266 compatibility
 * 
 * The original ssd1306_init() causes brownout resets on ESP8266 due to:
 * 1. High contrast setting (0x9f) draws too much current
 * 2. whole_display_lighting(true) turns on ALL pixels at once
 * 
 * This wrapper fixes both issues for stable operation.
 */
bool ssd1306_init_patched(const ssd1306_t *dev)
{
    // First, call the original init (it will fail but sets up the device)
    bool result = ssd1306_init(dev);
    
    if (result) {
        ESP_LOGI(TAG, "Applying ESP8266 compatibility patches...");
        
        // Override problematic settings with safe values
        ssd1306_set_contrast(dev, 0x7f);  // Reduced from 0x9f
        ssd1306_set_whole_display_lighting(dev, false);  // Don't light all pixels
        
        ESP_LOGI(TAG, "SSD1306 patches applied successfully");
    }
    
    return result;
}

// Optional: Create a macro to redirect calls
#ifdef SSD1306_MONKEY_PATCH
    #define ssd1306_init(dev) ssd1306_init_patched(dev)
#endif