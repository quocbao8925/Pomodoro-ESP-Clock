#include "ui.h"
#include "pomodoro.h"
#include "globals.h"
#include <stdio.h> 


void draw_classic_squix(u8g2_t *u8g2, squix_ui_t data, bool is_running_timer) {
    u8g2_ClearBuffer(u8g2);
    u8g2_SetFontDirection(u8g2, 0);

    // Split hour and minute strings
    char hour_str[3] = {data.time[0], data.time[1], '\0'};
    char min_str[3] = {data.time[3], data.time[4], '\0'};

    // Divider line and frame
    u8g2_DrawVLine(u8g2, 52, 0, 64); 
    u8g2_DrawFrame(u8g2, 0, 0, 128, 64); 
    
    // Left area: Clock
    u8g2_SetFont(u8g2, u8g2_font_logisoso24_tf); 
    u8g2_DrawStr(u8g2, 4, 28, hour_str);
    u8g2_DrawStr(u8g2, 4, 60, min_str);

    if (!data.is_24h) {
        u8g2_SetFont(u8g2, u8g2_font_5x8_tf); 
        u8g2_DrawStr(u8g2, 36, 60, data.is_pm ? "PM" : "AM");
    }
    if (is_running_timer) {
        u8g2_SetFont(u8g2, u8g2_font_open_iconic_app_1x_t); 
        u8g2_DrawGlyph(u8g2, 100, 62, 0x48);
    }
    // Right area: Weather and info
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(u8g2, 86, 14, data.date); 

    if (is_wifi_connected) {
        u8g2_SetFont(u8g2, u8g2_font_open_iconic_weather_2x_t);
        u8g2_DrawGlyph(u8g2, 60, 24, data.icon); 

        u8g2_SetFont(u8g2, u8g2_font_logisoso18_tf);
        char temp_buf[10];
        snprintf(temp_buf, 10, "%d", data.temp);
        u8g2_DrawUTF8(u8g2, 58, 46, temp_buf);
        u8g2_SetFont(u8g2, u8g2_font_5x8_tf);
        u8g2_DrawUTF8(u8g2, 86, 46, "°C");

        

        u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
        u8g2_DrawStr(u8g2, 86, 26, data.city); 
        
        char hum_buf[15];
        snprintf(hum_buf, 15, "H: %d%%", data.humidity);
        u8g2_DrawStr(u8g2, 58, 62, hum_buf);   
        
    } else {
        // Offline state
        
        u8g2_SetFont(u8g2, u8g2_font_open_iconic_www_2x_t);
        u8g2_DrawGlyph(u8g2, 60, 24, 0x4A);

        u8g2_SetFont(u8g2, u8g2_font_logisoso18_tf);
        u8g2_DrawUTF8(u8g2, 58, 46, "NA");
        u8g2_SetFont(u8g2, u8g2_font_5x8_tf);
        u8g2_DrawUTF8(u8g2, 86, 46, "°C");

        

        u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
        u8g2_DrawStr(u8g2, 86, 26, "OFFLINE"); 
        
        u8g2_DrawStr(u8g2, 58, 62, "H: NA%"); 
    }

    u8g2_SendBuffer(u8g2);
}

void draw_pomodoro_vertical(u8g2_t *u8g2, pomodoro_t *pomo, bool rotate, uint8_t loading_pct) {
    u8g2_ClearBuffer(u8g2);
    u8g2_DrawFrame(u8g2, 0, 0, 128, 64); 
    u8g2_SetFontDirection(u8g2, rotate ? 3 : 1); 
    
    if (rotate) {    
        // Work mode layout
        u8g2_SetFont(u8g2, u8g2_font_7x14B_tf);
        u8g2_DrawStr(u8g2, 110, 60, "WORKING");

        u8g2_SetFont(u8g2, u8g2_font_5x8_tf);
        u8g2_DrawStr(u8g2, 95, 60, "STATUS:");
        u8g2_DrawStr(u8g2, 120, 60, "POMODORO");

        u8g2_SetFont(u8g2, u8g2_font_6x12_tf);
        u8g2_DrawStr(u8g2, 60, 18, " 5");
        u8g2_DrawStr(u8g2, 40, 18, "25");

        uint8_t arrow_x = 40;
        u8g2_DrawTriangle(u8g2, arrow_x - 4, 1, arrow_x + 4, 1, arrow_x, 5);

        u8g2_SetFont(u8g2, u8g2_font_logisoso22_tf);
        
        char min_buf[5];
        snprintf(min_buf, 5, "%02d", pomo->minutes);
        u8g2_DrawStr(u8g2, 30, 62, min_buf); 

        char sec_buf[5];
        snprintf(sec_buf, 5, "%02d", pomo->seconds);
        u8g2_DrawStr(u8g2, 60, 62, sec_buf); 

        // Loading animation boxes
        if (loading_pct > 0 && loading_pct <= 100) {
            uint8_t box_w = 2;  
            uint8_t box_h = 8;  
            uint8_t space = 2;   
            uint8_t start_x = 72; 
            uint8_t start_y = 10; 

            if (loading_pct > 0) {
                uint8_t p1 = (loading_pct > 33) ? 33 : loading_pct;
                uint8_t fill_h1 = (p1 * box_h) / 33; 
                u8g2_DrawBox(u8g2, start_x, start_y, box_w, fill_h1);
            }
            if (loading_pct > 33) {
                uint8_t p2 = (loading_pct > 66) ? 33 : (loading_pct - 33);
                uint8_t fill_h2 = (p2 * box_h) / 33;
                u8g2_DrawBox(u8g2, start_x + box_w + space, start_y, box_w, fill_h2);
            }
            if (loading_pct > 66) {
                uint8_t p3 = (loading_pct > 100) ? 34 : (loading_pct - 66);
                uint8_t fill_h3 = (p3 * box_h) / 34;
                u8g2_DrawBox(u8g2, start_x + (box_w + space)*2, start_y, box_w, fill_h3);
            }
        }

        u8g2_SetFontDirection(u8g2, 0); 

        // Blinking colon indicator
        if (pomo->seconds % 2 == 0) {
            u8g2_DrawDisc(u8g2, 80, 62, 3, U8G2_DRAW_ALL); 
        }

        // Progress bar along edge
        uint8_t bar_length = (pomo->seconds * 64) / 60; 
        u8g2_DrawBox(u8g2, 0, 0, 3, bar_length); 

        // Flashing screen when timer ends
        if (pomo->flash_timer > 0) {
            if ((pomo->flash_timer / 2) % 2 == 0) {
                u8g2_SetDrawColor(u8g2, 2);        // XOR mode
                u8g2_DrawBox(u8g2, 0, 0, 128, 64); 
                u8g2_SetDrawColor(u8g2, 1);        // Normal mode
            }
        }

        u8g2_SendBuffer(u8g2); 
    } else {
        // Break mode layout
        u8g2_SetFont(u8g2, u8g2_font_7x14B_tf);
        u8g2_DrawStr(u8g2, 18, 4, "BREAKING");

        u8g2_SetFont(u8g2, u8g2_font_5x8_tf);
        u8g2_DrawStr(u8g2, 33, 4, "STATUS:");
        u8g2_DrawStr(u8g2, 8, 4, "POMODORO");

        u8g2_SetFont(u8g2, u8g2_font_6x12_tf);
        u8g2_DrawStr(u8g2, 68, 46, " 5");
        u8g2_DrawStr(u8g2, 88, 46, "25");

        uint8_t arrow_x = 60;
        u8g2_DrawTriangle(u8g2, 132 - arrow_x, 62, 124 - arrow_x, 62, 128 - arrow_x, 59);

        u8g2_SetFont(u8g2, u8g2_font_logisoso22_tf);
        
        char min_buf[5];
        snprintf(min_buf, 5, "%02d", pomo->minutes);
        u8g2_DrawStr(u8g2, 98, 2, min_buf); 

        char sec_buf[5];
        snprintf(sec_buf, 5, "%02d", pomo->seconds);
        u8g2_DrawStr(u8g2, 68, 2, sec_buf); 

        // Loading animation boxes
        if (loading_pct > 0 && loading_pct <= 100) {
            uint8_t box_w = 2;  
            uint8_t box_h = 8;  
            uint8_t space = 2;   
            uint8_t start_x = 48; 
            uint8_t start_y = 46; 

            if (loading_pct > 0) {
                uint8_t p1 = (loading_pct > 33) ? 33 : loading_pct;
                uint8_t fill_h1 = (p1 * box_h) / 33; 
                u8g2_DrawBox(u8g2, start_x, start_y, box_w, fill_h1);
            }
            if (loading_pct > 33) {
                uint8_t p2 = (loading_pct > 66) ? 33 : (loading_pct - 33);
                uint8_t fill_h2 = (p2 * box_h) / 33;
                u8g2_DrawBox(u8g2, start_x + box_w + space, start_y, box_w, fill_h2);
            }
            if (loading_pct > 66) {
                uint8_t p3 = (loading_pct > 100) ? 34 : (loading_pct - 66);
                uint8_t fill_h3 = (p3 * box_h) / 34;
                u8g2_DrawBox(u8g2, start_x + (box_w + space)*2, start_y, box_w, fill_h3);
            }
        }

        u8g2_SetFontDirection(u8g2, 0); 

        if (pomo->seconds % 2 == 0) {
            u8g2_DrawDisc(u8g2, 48, 1, 3, U8G2_DRAW_ALL); 
        }

        uint8_t bar_length = (pomo->seconds * 64) / 60; 
        u8g2_DrawBox(u8g2, 125, 64 - bar_length, 3, bar_length); 

        if (pomo->flash_timer > 0) {
            if ((pomo->flash_timer / 2) % 2 == 0) {
                u8g2_SetDrawColor(u8g2, 2);        
                u8g2_DrawBox(u8g2, 0, 0, 128, 64); 
                u8g2_SetDrawColor(u8g2, 1);        
            }
        }

        u8g2_SendBuffer(u8g2); 
    }
}

void draw_wifi_config_ui(u8g2_t *u8g2, bool is_serial_mode, uint8_t countdown, uint8_t loading_pct) {
    u8g2_ClearBuffer(u8g2);
    u8g2_SetFontDirection(u8g2, 2); 

    // 1. Tên thiết bị góc trái trên
    u8g2_SetFont(u8g2, u8g2_font_5x8_tf);
    u8g2_DrawStr(u8g2, 126, 56, "QBs-ESP32C3-MINI"); // 128-2=126, 64-8=56

    // 2. Logo WI CONFIG FI
    u8g2_SetFont(u8g2, u8g2_font_logisoso16_tf);
    u8g2_DrawStr(u8g2, 112, 28, "WI"); // 128-16=112, 64-36=28
    
    u8g2_SetFont(u8g2, u8g2_font_5x8_tf);
    u8g2_DrawStr(u8g2, 81, 34, "CONFIG"); // 128-47=81, 64-30=34
    
    u8g2_SetFont(u8g2, u8g2_font_logisoso16_tf);
    u8g2_DrawStr(u8g2, 112, 46, "FI"); // 128-16=112, 64-74 = -10 (Sẽ bị tràn ra ngoài màn hình)

    // 3. Số giây đếm ngược
    char buf[16];
    if (is_serial_mode) {
        snprintf(buf, sizeof(buf), "%02ds", countdown);
        u8g2_SetFont(u8g2, u8g2_font_7x14B_tf);
        u8g2_DrawStr(u8g2, 74, 12, buf); // 128-54=74, 64-52=12
    } else {
        u8g2_SetFont(u8g2, u8g2_font_5x8_tf);
        u8g2_DrawStr(u8g2, 93, 12, "PLUG USB & HOLD"); // 128-35=93, 64-52=12
    }

    // 4. Thanh Bar tiến trình (Đáy màn hình)
    if (is_serial_mode) {
        uint8_t w = (countdown * 128) / 45;
        u8g2_DrawBox(u8g2, 0, 4, w, 4); // 128-0=128, 64-60=4
    } else if (loading_pct > 0) {
        uint8_t w = (loading_pct * 128) / 100;
        u8g2_DrawBox(u8g2, 0, 4, w, 4); // 128-0=128, 64-60=4
    }

    u8g2_SendBuffer(u8g2);
}