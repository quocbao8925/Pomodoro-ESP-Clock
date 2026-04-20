#ifndef BUZZER_H
#define BUZZER_H

#include "driver/gpio.h"

// Các "Giai điệu" (Pattern) của Buzzer
typedef enum {
    BEEP_TEST,           
    BEEP_POMODORO_DONE,  // Kêu 5 tiếng bíp dài (Dành cho Pomodoro)
    BEEP_LONG,
} buzzer_pattern_t;

// Khởi tạo Buzzer trên 1 chân GPIO
void buzzer_init(gpio_num_t pin);

// Đặt mức âm lượng (0: Tắt, 1: Nhỏ, 2: Vừa, 3: To)
void buzzer_set_level(int level);

void buzzer_set_pitch(int pitch); // (Tùy chọn) Đặt tần số âm thanh

// Kích hoạt còi kêu (Hàm chạy bất đồng bộ, không block code)
void buzzer_play_pattern(buzzer_pattern_t pattern);

#endif