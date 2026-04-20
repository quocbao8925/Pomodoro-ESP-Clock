#ifndef POMODORO_H
#define POMODORO_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_timer.h"

// Cấu trúc gom nhóm toàn bộ dữ liệu của Pomodoro
typedef struct {
    int minutes;
    int seconds;
    esp_timer_handle_t timer_handle; // Biến quản lý hardware timer nằm luôn trong này
    int target_minutes; // 30, 25 hoặc 5
    bool is_running;
    bool is_break;
    int flash_timer;
    bool finish_pomo;
} pomodoro_t;

// Khai báo các hàm quản lý Pomodoro
void pomodoro_init(pomodoro_t *pomo);
// void pomodoro_handle_button(pomodoro_t *pomo, bool is_short_press, bool is_long_press);
void pomodoro_start_new(pomodoro_t *pomo, int target_mins);
void pomodoro_reset(pomodoro_t *pomo);
void pomodoro_stop(pomodoro_t *pomo);

#endif // POMODORO_H