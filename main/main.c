#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#include "u8g2.h"
#include "u8g2_esp32_hal.h"

#include "globals.h"
#include "app_tasks.h"
#include "buzzer.h"
#include "pomodoro.h"

#define BUZZER_PIN GPIO_NUM_4

// Khởi tạo các biến toàn cục (Đã khai báo extern trong globals.h)
pomodoro_t my_pomo_25 = {
    .minutes = 25,
    .seconds = 0,
    .is_running = false,
    .is_break = false,
    .flash_timer = 0,
    .target_minutes = 25
};
pomodoro_t my_pomo_5 = {
    .minutes = 5,
    .seconds = 0,
    .is_running = false,
    .is_break = true,
    .flash_timer = 0,
    .target_minutes = 5
};


u8g2_t u8g2;
SemaphoreHandle_t i2c_mutex;

int idle_seconds = 0;
bool is_display_off = false;
uint8_t current_screen = 0; 

bool is_24h_format = false;
uint8_t current_horizontal_ui = 0; 

int battery_percent = 100; 
int buzzer_level = 1;     
int buzzer_pitch = 2; 
int settings_selection = 0; 

void app_main(void) {

    // 2. Khởi tạo U8g2 (Màn hình OLED)
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.bus.i2c.sda = 8;
    u8g2_esp32_hal.bus.i2c.scl = 9;


    u8g2_esp32_hal_init(u8g2_esp32_hal);
    
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8x8_SetI2CAddress(&u8g2.u8x8, 0x3C << 1);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    // 3. Khởi tạo Buzzer
    buzzer_init(BUZZER_PIN);
    // buzzer_set_level(buzzer_level);

    // 4. Khởi tạo hệ thống Pomodoro
    pomodoro_init(&my_pomo_25);
    pomodoro_init(&my_pomo_5);

    // 5. Khởi chạy các Task song song của FreeRTOS
    // (Lưu ý: bmi160_init() sẽ tự động được gọi bên trong gyro_task)
    xTaskCreate(gyro_task, "gyro_task", 3072, NULL, 6, NULL);
    xTaskCreate(display_task, "display_task", 4096, NULL, 5, NULL);
}