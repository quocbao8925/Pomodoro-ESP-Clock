#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "u8g2.h"
#include "u8g2_esp32_hal.h"

static const char *TAG = "SMART_WATCH";

// --- BIẾN LƯU TRONG RTC RAM (Không mất khi Deep Sleep) ---
RTC_DATA_ATTR int boot_count = 0;
RTC_DATA_ATTR struct {
    int p_minutes;      // Phút còn lại của Pomodoro
    int p_seconds;      // Giây còn lại
    bool is_working;    // Đang làm việc hay đang nghỉ
    float last_temp;    // Nhiệt độ lần cuối cập nhật
    uint32_t last_weather_sync; // Timestamp lần cuối sync thời tiết
} sys_state;

// --- PHẦN 1: CÁC TASK CHỨC NĂNG (Để trống logic) ---

void task_update_clock() {
    ESP_LOGI(TAG, "Task: Cap nhat thoi gian tu RTC/NTP");
    // TODO: Lay thoi gian hien tai va ve len buffer U8g2
}

void task_update_weather() {
    // Kiem tra neu da den luc (30p/lan) thi moi bat WiFi
    ESP_LOGI(TAG, "Task: HTTP Request lay thoi tiet");
    // TODO: Ket noi WiFi -> HTTP Get -> Parse JSON -> Luu vao sys_state.last_temp
}

void task_pomodoro_logic() {
    ESP_LOGI(TAG, "Task: Tinh toan thoi gian Pomodoro");
    // TODO: Tru di 1 phut (vi moi phut thuc day 1 lan)
}

void task_draw_ui(u8g2_t *u8g2) {
    void draw_smartwatch_ui(u8g2_t *u8g2, smartwatch_data_t data) {
    u8g2_ClearBuffer(u8g2);
    
    // 1. Vẽ Giờ (Góc trên bên trái)
    u8g2_SetFont(u8g2, u8g2_font_logisoso18_tf); // Font 16px
    u8g2_DrawStr(u8g2, 6, 18, data.time_str);

    // 2. Vẽ Ngày tháng (Góc trên bên phải)
    u8g2_SetFont(u8g2, u8g2_font_6x12_tf);       // Font 10px
    u8g2_DrawStr(u8g2, 85, 18, data.date_str);

    // 3. Đường kẻ ngang phân cách
    u8g2_DrawHLine(u8g2, 0, 22, 128);            // Kẻ từ X=0 đến 128 ở độ cao Y=22
    
    // 4. Icon thời tiết (Góc dưới bên trái)
    u8g2_SetFont(u8g2, u8g2_font_open_iconic_weather_2x_t); // Size 32x32
    u8g2_DrawGlyph(u8g2, 4, 50, data.weather_icon); 

    // 5. Nhiệt độ (Ở giữa)
    u8g2_SetFont(u8g2, u8g2_font_logisoso18_tf); // Font 22px
    char temp_str[10];
    sprintf(temp_str, "%d°C", data.temp);
    u8g2_DrawUTF8(u8g2, 22, 56, temp_str);       // Dùng UTF8 để hiện dấu độ

    // 6. Thông tin phụ (Góc dưới bên phải)
    u8g2_SetFont(u8g2, u8g2_font_6x12_tf);       // Font 10px
    u8g2_DrawStr(u8g2, 85, 42, data.city);
    
    char hum_str[15];
    sprintf(hum_str, "H: %d%%", data.humidity);
    u8g2_DrawStr(u8g2, 85, 56, hum_str);

    u8g2_SendBuffer(u8g2);
}
}

// --- PHẦN 2: QUẢN LÝ NGUỒN VÀ WAKEUP ---

void check_wakeup_reason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_TIMER: 
            ESP_LOGI(TAG, "Thuc day do hen gio (Update moi phut)"); 
            break;
        case ESP_SLEEP_WAKEUP_GPIO: 
            ESP_LOGI(TAG, "Thuc day do Nut bam/Gyro"); 
            break;
        default: 
            ESP_LOGI(TAG, "Thuc day lan dau (Reset)");
            // Khoi tao gia tri mac dinh cho sys_state o day
            break;
    }
}

// --- PHẦN 3: MAIN ENTRY POINT ---

void app_main(void) {
    boot_count++;
    
    // 1. Khoi tao phan cung (I2C, OLED)
    // TODO: Setup U8g2 HAL và Hardware
    u8g2_t u8g2; 
    // ... (Code khoi tao u8g2) ...

    // 2. Kiem tra ly do thuc day
    check_wakeup_reason();

    // 3. Thực thi các "Task" theo trình tự
    task_update_clock();
    task_pomodoro_logic();
    
    // Chỉ cập nhật thời tiết mỗi 60 phút hoặc khi bấm nút
    if (boot_count % 60 == 0) {
        task_update_weather();
    }

    // 4. Vẽ màn hình lần cuối trước khi ngủ
    task_draw_ui(&u8g2);

    // 5. Chuan bi di ngu
    ESP_LOGI(TAG, "Chuan bi Deep Sleep trong 60 giay...");
    
    // Thuc day sau 1 phut
    esp_sleep_enable_timer_wakeup(60 * 1000000); 
    
    // Thuc day bang Nut bam (GPIO 0)
    esp_deep_sleep_enable_gpio_wakeup(1 << GPIO_NUM_0, ESP_GPIO_WAKEUP_GPIO_LOW);

    // Tat man hinh OLED de tiet kiem dien
    // u8g2_SetPowerSave(&u8g2, 1); 

    esp_deep_sleep_start();
}