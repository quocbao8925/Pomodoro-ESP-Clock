#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "u8g2.h"
#include "pomodoro.h"
#include "sdkconfig.h"



extern bool is_wifi_connected;

extern int8_t current_temp;
extern uint16_t weather_icon_code;

extern pomodoro_t my_pomo_25;
extern pomodoro_t my_pomo_5;
extern u8g2_t u8g2;


extern uint16_t idle_seconds;
extern bool is_display_off;
extern uint8_t current_screen; 

extern bool is_24h_format;
extern uint8_t current_horizontal_ui;

extern int settings_selection;

void wake_up_display(void);
void test_buzzer(void);

#endif // GLOBALS_H