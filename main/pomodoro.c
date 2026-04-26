#include "pomodoro.h"
#include "buzzer.h"
#include <stddef.h>

// Hardware timer callback triggered every 1 second.
static void pomodoro_timer_callback(void* arg) {
    pomodoro_t *pomo = (pomodoro_t*)arg;
    if (pomo->finish_pomo) return;

    if (pomo->seconds == 0) {
        if (pomo->minutes == 0) {
            pomo->finish_pomo = true;
            esp_timer_stop(pomo->timer_handle);
            pomo->is_running = false;
            pomo->flash_timer = 30; 
            buzzer_play_pattern(BEEP_POMODORO_DONE);
        } else {
            pomo->minutes--;
            pomo->seconds = 59;
        }
    } else {
        pomo->seconds--;
    }
}

void pomodoro_init(pomodoro_t *pomo) {
    pomo->minutes    = pomo->target_minutes;
    pomo->seconds    = 0;
    pomo->is_running = false;
    pomo->is_break   = (pomo->target_minutes == 5);
    pomo->flash_timer  = 0;
    pomo->finish_pomo  = false;

    const esp_timer_create_args_t timer_args = {
        .callback = &pomodoro_timer_callback,
        .arg      = pomo, 
        .name     = "pomo_timer"
    };
    esp_timer_create(&timer_args, &pomo->timer_handle);
}

void pomodoro_reset(pomodoro_t *pomo) {
    if (pomo->is_running) {
        esp_timer_stop(pomo->timer_handle);
    }
    pomo->is_running  = false;
    pomo->minutes     = pomo->target_minutes;
    pomo->seconds     = 0;
    pomo->is_break    = (pomo->target_minutes == 5);
    pomo->flash_timer = 0; 
    pomo->finish_pomo = false; 
}

void pomodoro_start_new(pomodoro_t *pomo, uint8_t target_mins) {
    if (pomo->is_running) {
        esp_timer_stop(pomo->timer_handle);
    }
    pomo->target_minutes = target_mins;
    pomo->minutes     = target_mins;
    pomo->seconds     = 0;
    pomo->is_break    = (target_mins == 5);
    pomo->flash_timer = 0;
    pomo->is_running  = true;
    esp_timer_start_periodic(pomo->timer_handle, 1000000); 
}