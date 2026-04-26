#ifndef BUZZER_H
#define BUZZER_H

#include "driver/gpio.h"

typedef enum {
    BEEP_TEST,           
    BEEP_POMODORO_DONE, 
    BEEP_LONG,
    BEEP_SHORT,
    BEEP_TWICE
} buzzer_pattern_t;

void buzzer_init(gpio_num_t pin);

void buzzer_play_pattern(buzzer_pattern_t pattern);

#endif