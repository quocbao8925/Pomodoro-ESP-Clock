#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "u8g2.h"
#include "pomodoro.h"


// 1. Cấu trúc dữ liệu cho Smartwatch
typedef struct {
    char time[10];
    char date[15];
    char city[10];
    int temp;
    int humidity;
    uint8_t icon;
    bool is_24h; // Thêm cờ định dạng giờ
    bool is_pm;  // Thêm cờ sáng/chiều
} squix_ui_t;

typedef struct {
    char time_str[10];
    char date_str[15];
    char city[10];
    int temp;
    int humidity;
    uint8_t weather_icon;
    bool is_24h; // Thêm cờ định dạng giờ
    bool is_pm;  // Thêm cờ sáng/chiều
} smartwatch_data_t;

// Khai báo hàm mới
void draw_settings_vertical(u8g2_t *u8g2, int bat_percent, int buzzer_level, int buzzer_pitch, int settings_selection);
void draw_classic_squix(u8g2_t *u8g2, squix_ui_t data);
void draw_smartwatch_ui(u8g2_t *u8g2, smartwatch_data_t data);
void draw_pomodoro_vertical(u8g2_t *u8g2, pomodoro_t *pomo, bool rotate, int loading_pct);

#endif // UI_H