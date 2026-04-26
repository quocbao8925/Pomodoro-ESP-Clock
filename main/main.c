#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_pm.h"

#include "u8g2.h"
#include "u8g2_esp32_hal.h"

#include "globals.h"
#include "app_tasks.h"
#include "buzzer.h"
#include "pomodoro.h"

#include "nvs_flash.h"

#define BUZZER_PIN GPIO_NUM_4

// Initialize global variables (extern in globals.h)
pomodoro_t my_pomo_25 = {
    .minutes = 25,
    .seconds = 0,
    .target_minutes = 25,
    .flash_timer = 0,
    .is_running = false,
    .is_break = false
};

pomodoro_t my_pomo_5 = {
    .minutes = 5,
    .seconds = 0,
    .target_minutes = 5,
    .flash_timer = 0,
    .is_running = false,
    .is_break = true
};

u8g2_t u8g2;
SemaphoreHandle_t i2c_mutex;

uint16_t idle_seconds = 0;
bool is_display_off = false;
uint8_t current_screen = 0; 

bool is_24h_format = false;
uint8_t current_horizontal_ui = 0; 


void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    esp_pm_config_t pm_config = {
        .max_freq_mhz       = 80,   // 80 MHz is enough for I2C, display, JSON parsing.
        .min_freq_mhz       = 60,   // Drop to 60 MHz when fully idle.
        .light_sleep_enable = true, // Auto light sleep between task wakeups.
    };
    esp_pm_configure(&pm_config);
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.bus.i2c.sda = 8;
    u8g2_esp32_hal.bus.i2c.scl = 9;

    //u8g2_esp32_hal.bus.i2c.clk_speed = 400000;

    u8g2_esp32_hal_init(u8g2_esp32_hal);
    
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8x8_SetI2CAddress(&u8g2.u8x8, 0x3C << 1);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    buzzer_init(BUZZER_PIN);

    pomodoro_init(&my_pomo_25);
    pomodoro_init(&my_pomo_5);

    xTaskCreate(gyro_task, "gyro_task", 3072, NULL, 6, NULL);
    xTaskCreate(display_task, "display_task", 4096, NULL, 5, NULL);
    xTaskCreate(wifi_sync_task, "wifi_sync_task", 4096, NULL, 4, NULL);
    xTaskCreate(serial_listen_task, "serial_task", 4096, NULL, 3, NULL);
}