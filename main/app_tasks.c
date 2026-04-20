#include "app_tasks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "u8g2_esp32_hal.h"

#include "globals.h"
#include "bmi160.h"
#include "buzzer.h"
#include "ui.h"
#include "pomodoro.h"

#define BUTTON_PIN GPIO_NUM_10

int loading_progress = 0; 

void wake_up_display() {
    idle_seconds = 0; 
    
    if (is_display_off) {
        u8g2_SetPowerSave(&u8g2, 0); 
        is_display_off = false;
        printf("Display turned on\n");
    }
}

static int test = 0;
void test_buzzer() {
    if (!test){
        test = 1;
        buzzer_play_pattern(BEEP_TEST);
    }
}
void long_beep() {
    buzzer_play_pattern(BEEP_LONG);
}

void gyro_task(void *pvParameter) {
    bmi160_init();

    int16_t sleep_accel_z = 0;   
    bool was_display_off = false; 

    int loading_ms = 0;
    int target_mode = 0;

    int last_screen = 0;

    while(1) {
        int16_t accel_y = 0, accel_z = 0;
        
        accel_y = bmi160_read_accel_y();
        accel_z = bmi160_read_accel_z(); 
        
        //printf("BMI160 Data -> Y: %d \t Z: %d\n", accel_y, accel_z);

        if (is_display_off && !was_display_off) {
            sleep_accel_z = accel_z;
        }

        if (is_display_off) {
            if (abs(accel_z - sleep_accel_z) > 1000) {
                wake_up_display();
            }
        }

        was_display_off = is_display_off;

        int new_screen = current_screen;

        if (accel_z > 8000) {
            // --- MÀN HÌNH 25 PHÚT (WORK) ---
            if (!my_pomo_25.is_running && !my_pomo_25.finish_pomo) {
                my_pomo_25.minutes = 25; 
                my_pomo_25.seconds = 0;
            }
            current_screen = 1; 
            new_screen = 1;

            // Xử lý màn hình 25 phút
            if (my_pomo_25.is_running || my_pomo_25.finish_pomo) {
                // Đang đếm ngược HOẶC đã đếm xong -> Không làm gì cả
                target_mode = 0; 
                loading_ms = 0; 
            } else {
                // Đang ở trạng thái chờ -> Kích hoạt loading 3 giây
                target_mode = 25;
            }
        } 
        else if (accel_z < -8000) {
            // --- MÀN HÌNH 5 PHÚT (BREAK) ---
            if (!my_pomo_5.is_running && !my_pomo_5.finish_pomo) {
                my_pomo_5.minutes = 5; 
                my_pomo_5.seconds = 0;
            }
            current_screen = 2;
            new_screen = 2;
            
            // Xử lý màn hình 5 phút
            if (my_pomo_5.is_running || my_pomo_5.finish_pomo) {
                target_mode = 0; 
                loading_ms = 0;
            } else {
                target_mode = 5;
            }
        } 
        else {
            // --- MÀN HÌNH CHÍNH (CLOCK) ---
            current_screen = 0; 
            new_screen = 0;
            test = 0;
            target_mode = 0; 
            loading_ms = 0;
            
        }
        if (last_screen == 1 && new_screen != 1) {
            // Sự kiện: Vừa lật tay thoát khỏi màn hình 25 phút
            if (my_pomo_25.finish_pomo) {
                pomodoro_reset(&my_pomo_25);
            }
        }
        if (last_screen == 2 && new_screen != 2) {
            // Sự kiện: Vừa lật tay thoát khỏi màn hình 5 phút
            if (my_pomo_5.finish_pomo) {
                pomodoro_reset(&my_pomo_5);
            }
        }
        last_screen = new_screen;
        current_screen = new_screen;
        // --- XỬ LÝ LOADING 3 GIÂY & START ---
        if (target_mode != 0 && !is_display_off) {
            loading_ms += 200; // gyro_task lặp mỗi 200ms
            loading_progress = (loading_ms * 100) / 3000 + 30; // Tính %

            if (loading_ms >= 3000) {
                // Đủ 3 giây -> Bắt đầu đếm ngược! 
                // LÚC NÀY MỚI CHÍNH THỨC RESET BỘ ĐẾM CÒN LẠI
                if (target_mode == 25) {
                    pomodoro_start_new(&my_pomo_25, target_mode);
                    pomodoro_reset(&my_pomo_5); // Dập tắt 5 phút
                } else if (target_mode == 5) {
                    pomodoro_start_new(&my_pomo_5, target_mode);
                    pomodoro_reset(&my_pomo_25); // Dập tắt 25 phút
                }
                long_beep();
                
                target_mode = 0;
                loading_ms = 0;
                loading_progress = 0;
            }
        } else {
            loading_progress = 0; // Trượt tay lật đi hướng khác -> Hủy loading
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}


void display_task(void *pvParameter) {
    while(1) {
        if (my_pomo_5.is_running) {
            idle_seconds = 0; 
        } else {
            static int loop_count = 0;
            loop_count++;
            if (loop_count >= 10) {
                idle_seconds++;
                loop_count = 0;
            }
        }

        if (idle_seconds > 15 && !is_display_off) {
            u8g2_SetPowerSave(&u8g2, 1); 
            is_display_off = true;
            printf("Display sleep\n");
        }

        int hour = 14; 
        int min = 40;
        bool is_pm = false;
        
        if (!is_24h_format) {
            if (hour >= 12) {
                is_pm = true;
                if (hour > 12) hour -= 12;
            } else {
                is_pm = false;
                if (hour == 0) hour = 12;
            }
        }
        char time_str[10];
        sprintf(time_str, "%02d:%02d", hour, min);

        if (!is_display_off) {
            if (current_screen == 0) {
                if (current_horizontal_ui == 0) {
                    squix_ui_t squix_data = {
                        .date = "10 Apr", .city = "HCMC", .temp = 34, .humidity = 60, .icon = 0x43,
                        .is_24h = is_24h_format, .is_pm = is_pm
                    };
                    strcpy(squix_data.time, time_str);
                    draw_classic_squix(&u8g2, squix_data);
                } else {
                    smartwatch_data_t smart_data = {
                        .date_str = "10 Apr", .city = "HCMC", .temp = 34, .humidity = 60, .weather_icon = 0x43,
                        .is_24h = is_24h_format, .is_pm = is_pm
                    };
                    strcpy(smart_data.time_str, time_str);
                    draw_smartwatch_ui(&u8g2, smart_data);
                }
            } 
            else if (current_screen == 1) {
                draw_pomodoro_vertical(&u8g2, &my_pomo_25, 1, loading_progress);
            }
            else if (current_screen == 2) {
                draw_pomodoro_vertical(&u8g2, &my_pomo_5, 0, loading_progress);
            } 
            else if (current_screen == 3) {
                u8g2_ClearBuffer(&u8g2);
                u8g2_SendBuffer(&u8g2);
                
            }
        }
        
        if (my_pomo_25.flash_timer > 0) {
            wake_up_display(); 
            my_pomo_25.flash_timer--;
        } else if (my_pomo_5.flash_timer > 0) {
            wake_up_display(); 
            my_pomo_5.flash_timer--;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}