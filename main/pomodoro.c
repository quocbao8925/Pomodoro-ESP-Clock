#include "pomodoro.h"
#include "buzzer.h"
#include <stddef.h>

// Hàm Callback được gọi ngầm bởi phần cứng mỗi 1 giây
// Lưu ý: Từ khóa 'static' giúp hàm này chỉ hoạt động nội bộ trong file pomodoro.c
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

// Trong hàm pomodoro_init, đừng quên set nó về 0
void pomodoro_init(pomodoro_t *pomo) {
    pomo->target_minutes = 25; 
    pomo->minutes = 25;
    pomo->seconds = 0;
    pomo->is_running = false;
    pomo->is_break = false;
    pomo->flash_timer = 0;
    pomo->finish_pomo = false;

    const esp_timer_create_args_t timer_args = {
        .callback = &pomodoro_timer_callback,
        .arg = pomo, 
        .name = "pomo_timer"
    };
    esp_timer_create(&timer_args, &pomo->timer_handle);
}


// void pomodoro_handle_button(pomodoro_t *pomo, bool is_short_press, bool is_long_press) {
//     if (is_short_press && !pomo->is_running) {
//         // --- Xoay vòng mốc thời gian: 30 -> 25 -> 5 ---
//         if (pomo->target_minutes == 30) pomo->target_minutes = 25;
//         else if (pomo->target_minutes == 25) pomo->target_minutes = 5;
//         else pomo->target_minutes = 30;

//         // Cập nhật ngay lên màn hình
//         pomo->minutes = pomo->target_minutes;
//         pomo->seconds = 0;
//         pomo->is_break = (pomo->target_minutes == 5);
//     } 
//     else if (is_long_press) {
//         pomo->is_running = !pomo->is_running;
//         if (pomo->is_running) {
//             esp_timer_start_periodic(pomo->timer_handle, 1000000); 
//         } else {
//             esp_timer_stop(pomo->timer_handle); 
//         }
//     }
// }
void pomodoro_reset(pomodoro_t *pomo) {
    if (pomo->is_running) {
        esp_timer_stop(pomo->timer_handle);
    }
    pomo->is_running = false;
    pomo->minutes = pomo->target_minutes;
    pomo->seconds = 0;
    pomo->is_break = (pomo->target_minutes == 5);
    pomo->flash_timer = 0; // Reset nhấp nháy
    
    // Đảm bảo xóa cờ hoàn thành để sẵn sàng cho lần sau
    pomo->finish_pomo = false; 
}
void pomodoro_start_new(pomodoro_t *pomo, int target_mins) {
    if (pomo->is_running) {
        esp_timer_stop(pomo->timer_handle);
    }
    pomo->target_minutes = target_mins;
    pomo->minutes = target_mins;
    pomo->seconds = 0;
    pomo->is_break = (target_mins == 5);
    pomo->flash_timer = 0;
    pomo->is_running = true;
    esp_timer_start_periodic(pomo->timer_handle, 1000000); // Bắt đầu đếm ngay
}

