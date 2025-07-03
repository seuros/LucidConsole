/*
 * ESP8266 Hardware I2C Bus Driver
 * 
 * Implementation based on ESP8266 RTOS SDK I2C example
 */

#include "i2c_hw_bus.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/spi.h"
#include "esp8266/pin_mux_register.h"

static const char* TAG = "I2C_HW";

// FreeRTOS I2C bus mutex for thread-safe access
static SemaphoreHandle_t i2c_mutex = NULL;

esp_err_t i2c_hw_bus_init(void) {
    // Initialize ESP8266 hardware I2C bus
    
    // CRITICAL: De-initialize HSPI to free GPIO12/14 for I2C use
    // De-initialize HSPI to free GPIO12/14
    esp_err_t ret = spi_deinit(HSPI_HOST);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "HSPI deinit warning: %s", esp_err_to_name(ret));
    }
    
    // CRITICAL: Re-mux GPIO12/14 from HSPI pins to GPIO mode
    // Re-mux GPIO12/14 from HSPI to GPIO mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);  // GPIO12 to GPIO mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);  // GPIO14 to GPIO mode
    vTaskDelay(pdMS_TO_TICKS(10)); // Small delay for pin mux to settle
    
    // Configure I2C parameters
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,        // GPIO12
        .sda_pullup_en = 1,                     // Enable internal pullup
        .scl_io_num = I2C_MASTER_SCL_IO,        // GPIO14
        .scl_pullup_en = 1,                     // Enable internal pullup
        .clk_stretch_tick = 300,                // Clock stretch timeout (210us)
    };
    
    // Install I2C driver
    ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2C driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Configure I2C parameters
    ret = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2C parameters: %s", esp_err_to_name(ret));
        i2c_driver_delete(I2C_MASTER_NUM);
        return ret;
    }
    
    // Create FreeRTOS mutex for thread-safe I2C access
    i2c_mutex = xSemaphoreCreateMutex();
    if (i2c_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create I2C mutex");
        i2c_driver_delete(I2C_MASTER_NUM);
        return ESP_ERR_NO_MEM;
    }
    
    ESP_LOGI(TAG, "I2C hardware bus initialized");
    ESP_LOGI(TAG, "   - SDA: GPIO%d (with pullup)", I2C_MASTER_SDA_IO);
    ESP_LOGI(TAG, "   - SCL: GPIO%d (with pullup)", I2C_MASTER_SCL_IO);
    ESP_LOGI(TAG, "   - Frequency: %d Hz", I2C_MASTER_FREQ_HZ);
    ESP_LOGI(TAG, "   - FreeRTOS mutex created for thread safety");
    
    return ESP_OK;
}

esp_err_t i2c_hw_write_cmd(uint8_t device_addr, uint8_t reg_addr, uint8_t command) {
    return i2c_hw_write_data(device_addr, reg_addr, &command, 1);
}

esp_err_t i2c_hw_write_data(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, size_t data_len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    // Build I2C transaction
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_write(cmd, data, data_len, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    
    // Execute transaction
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C write failed to device 0x%02x reg 0x%02x: %s", 
                device_addr, reg_addr, esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t i2c_hw_read_data(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, size_t data_len) {
    esp_err_t ret;
    
    // Write register address
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C write register address failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Read data
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | READ_BIT, ACK_CHECK_EN);
    if (data_len > 1) {
        i2c_master_read(cmd, data, data_len - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data + data_len - 1, NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C read failed from device 0x%02x reg 0x%02x: %s", 
                device_addr, reg_addr, esp_err_to_name(ret));
    }
    
    return ret;
}

int i2c_hw_scan_devices(void) {
    ESP_LOGI(TAG, "I2C bus scan started");
    
    int devices_found = 0;
    esp_err_t ret;
    
    for (uint8_t addr = 0x08; addr < 0x78; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        
        ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 50 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found I2C device at address 0x%02x", addr);
            devices_found++;
        }
        
        vTaskDelay(10 / portTICK_RATE_MS);
    }
    
    ESP_LOGI(TAG, "I2C scan complete: %d devices found", devices_found);
    return devices_found;
}

esp_err_t i2c_hw_bus_lock(uint32_t timeout_ms) {
    if (i2c_mutex == NULL) {
        ESP_LOGE(TAG, "I2C mutex not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    TickType_t timeout_ticks = timeout_ms / portTICK_PERIOD_MS;
    if (xSemaphoreTake(i2c_mutex, timeout_ticks) != pdTRUE) {
        ESP_LOGW(TAG, "I2C bus lock timeout after %ums", timeout_ms);
        return ESP_ERR_TIMEOUT;
    }
    
    return ESP_OK;
}

void i2c_hw_bus_unlock(void) {
    if (i2c_mutex == NULL) {
        ESP_LOGE(TAG, "I2C mutex not initialized");
        return;
    }
    
    xSemaphoreGive(i2c_mutex);
}