#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "u8g2.h"
#include "pomodoro.h"


typedef struct {
    char time[10];
    char date[15];
    char city[10];
    uint8_t temp;
    uint8_t humidity;
    uint8_t icon;
    bool is_24h; // time format
    bool is_pm; // AM/PM indicator
} squix_ui_t;


void draw_classic_squix(u8g2_t *u8g2, squix_ui_t data, bool is_running_timer);
void draw_pomodoro_vertical(u8g2_t *u8g2, pomodoro_t *pomo, bool rotate, uint8_t loading_pct);
void draw_wifi_config_ui(u8g2_t *u8g2, bool is_serial_mode, uint8_t countdown, uint8_t loading_pct);
#endif // UI_H