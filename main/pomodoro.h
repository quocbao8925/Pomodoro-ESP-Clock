#ifndef POMODORO_H
#define POMODORO_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_timer.h"

typedef struct {
    uint8_t minutes;
    uint8_t seconds;
    esp_timer_handle_t timer_handle;
    uint8_t target_minutes;
    bool is_running;
    bool is_break;
    uint8_t flash_timer;
    bool finish_pomo;
} pomodoro_t;

void pomodoro_init(pomodoro_t *pomo);

void pomodoro_start_new(pomodoro_t *pomo, uint8_t target_mins);
void pomodoro_reset(pomodoro_t *pomo);
void pomodoro_stop(pomodoro_t *pomo);

#endif // POMODORO_H