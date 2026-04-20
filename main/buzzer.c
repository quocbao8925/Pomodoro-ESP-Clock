#include "buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/ledc.h"

#define BUZZER_LEDC_TIMER       LEDC_TIMER_0
#define BUZZER_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_CHANNEL     LEDC_CHANNEL_0
#define BUZZER_DUTY_RES         LEDC_TIMER_13_BIT // Độ phân giải 8192

#define BUZZER_DUTY             512
#define BUZZER_FREQ             2500 

// static int current_level = 1;
// static int current_pitch = 2; // (Tùy chọn) Biến lưu tần số âm thanh hiện tại
static QueueHandle_t buzzer_queue;

// Hàm nội bộ: Chuyển mức 1,2,3 thành Duty Cycle của PWM
// static uint32_t get_duty_for_level(int level) {
//     switch(level) {
//         case 1: return 100;   // Nhỏ (Xung hẹp)
//         case 2: return 800;   // Vừa
//         case 3: return 4096;  // To nhất (Xung vuông 50%)
//         default: return 2000;    // Mute
//     }
// }

// static uint32_t get_freq_for_pitch(int pitch) {
//     switch(pitch) {
//         case 1: return 1000;  // Âm trầm (1000 Hz) - Nghe êm ái
//         case 2: return 2500;  // Âm vừa (2500 Hz) - Chuẩn báo thức
//         case 3: return 4000;  // Âm cao (4000 Hz) - Rất gắt và chói tai
//         default: return 2500; 
//     }
// }

// void buzzer_set_pitch(int pitch) {
//     if (pitch >= 1 && pitch <= 3) {
//         current_pitch = pitch;
//     }
// }

static void buzzer_task(void *arg) {
    buzzer_pattern_t pattern;
    
    while(1) {
        if (xQueueReceive(buzzer_queue, &pattern, portMAX_DELAY)) {

            uint32_t duty = BUZZER_DUTY;
            uint32_t freq = BUZZER_FREQ;
            
            ledc_set_freq(BUZZER_LEDC_MODE, BUZZER_LEDC_TIMER, freq);
            
            if (pattern == BEEP_POMODORO_DONE || pattern == BEEP_TEST) {
                // Lặp 3 lần: Beep - Beep - Nghỉ
                for (int i = 0; i < 8; i++) {
                    ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, duty);
                    ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                    vTaskDelay(pdMS_TO_TICKS(80)); // Beep 1
                    
                    ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 0);
                    ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                    vTaskDelay(pdMS_TO_TICKS(60)); // Ngắt
                    
                    ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, duty);
                    ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                    vTaskDelay(pdMS_TO_TICKS(80)); // Beep 2
                    
                    ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 0);
                    ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                    vTaskDelay(pdMS_TO_TICKS(400)); // Nghỉ dài
                }
            } else if (pattern == BEEP_LONG) {
                // Beep dài 1 giây
                ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, duty);
                ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                vTaskDelay(pdMS_TO_TICKS(800));
                
                ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 0);
                ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
            }
        }
    }
}

void buzzer_init(gpio_num_t pin) {
    // 1. Cấu hình Timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = BUZZER_LEDC_MODE,
        .timer_num        = BUZZER_LEDC_TIMER,
        .duty_resolution  = BUZZER_DUTY_RES,
        .freq_hz          = 2500,  // Tần số âm thanh (2500Hz nghe khá chói và rõ)
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // 2. Cấu hình Channel liên kết với chân GPIO
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = BUZZER_LEDC_MODE,
        .channel        = BUZZER_LEDC_CHANNEL,
        .timer_sel      = BUZZER_LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = pin,
        .duty           = 0, // Bắt đầu ở trạng thái tắt
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);

    // 3. Khởi tạo Queue (sức chứa 5 lệnh) và Task
    buzzer_queue = xQueueCreate(5, sizeof(buzzer_pattern_t));
    xTaskCreate(buzzer_task, "buzzer_task", 2048, NULL, 5, NULL);
}
// void buzzer_set_level(int level) {
//     current_level = level;
// }

void buzzer_play_pattern(buzzer_pattern_t pattern) {
    if (buzzer_queue != NULL) {
        // Ném lệnh vào hàng đợi (Không block code chính)
        xQueueSend(buzzer_queue, &pattern, 0);
    }
}