#include "buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/ledc.h"

#define BUZZER_LEDC_TIMER       LEDC_TIMER_0
#define BUZZER_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_CHANNEL     LEDC_CHANNEL_0
#define BUZZER_DUTY_RES         LEDC_TIMER_13_BIT // 8192 resolution

#define BUZZER_DUTY             512
#define BUZZER_FREQ             2500 

static QueueHandle_t buzzer_queue;

static void buzzer_task(void *arg) {
    buzzer_pattern_t pattern;
    
    while(1) {
        if (xQueueReceive(buzzer_queue, &pattern, portMAX_DELAY)) {

            uint32_t duty = BUZZER_DUTY;
            uint32_t freq = BUZZER_FREQ;
            
            ledc_set_freq(BUZZER_LEDC_MODE, BUZZER_LEDC_TIMER, freq);
            
            switch (pattern) {
                case BEEP_TEST:
                    for (uint8_t i = 0; i < 8; i++) {
                        ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, duty);
                        ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                        vTaskDelay(pdMS_TO_TICKS(80)); 
                        
                        ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 0);
                        ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                        vTaskDelay(pdMS_TO_TICKS(60)); 
                        
                        ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, duty);
                        ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                        vTaskDelay(pdMS_TO_TICKS(80)); 
                        
                        ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 0);
                        ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                        vTaskDelay(pdMS_TO_TICKS(400)); 
                    }
                    break;
                case BEEP_POMODORO_DONE:
                    for (uint8_t i = 0; i < 8; i++) {
                        ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, duty);
                        ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                        vTaskDelay(pdMS_TO_TICKS(80)); 
                        
                        ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 0);
                        ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                        vTaskDelay(pdMS_TO_TICKS(60)); 
                        
                        ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, duty);
                        ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                        vTaskDelay(pdMS_TO_TICKS(80)); 
                        
                        ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 0);
                        ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                        vTaskDelay(pdMS_TO_TICKS(400)); 
                    }
                    break;
                case BEEP_LONG:
                    ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, duty);
                    ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                    vTaskDelay(pdMS_TO_TICKS(800));
                    
                    ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 0);
                    ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    break;
                case BEEP_SHORT:
                    ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, duty);
                    ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    
                    ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 0);
                    ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    break;
                case BEEP_TWICE:
                    for (uint8_t i = 0; i < 2; i++) {
                        ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, duty);
                        ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                        vTaskDelay(pdMS_TO_TICKS(100)); 
                        
                        ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 0);
                        ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
                        vTaskDelay(pdMS_TO_TICKS(80)); 
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

void buzzer_init(gpio_num_t pin) {
    // 1. Timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = BUZZER_LEDC_MODE,
        .timer_num        = BUZZER_LEDC_TIMER,
        .duty_resolution  = BUZZER_DUTY_RES,
        .freq_hz          = BUZZER_FREQ,  
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // 2. Channel configuration linked to GPIO pin
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = BUZZER_LEDC_MODE,
        .channel        = BUZZER_LEDC_CHANNEL,
        .timer_sel      = BUZZER_LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = pin,
        .duty           = 0, // Start in off state
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);

    // 3. Initialize Queue (capacity of 5 commands) and Task
    buzzer_queue = xQueueCreate(5, sizeof(buzzer_pattern_t));
    xTaskCreate(buzzer_task, "buzzer_task", 2048, NULL, 5, NULL);
}

void buzzer_play_pattern(buzzer_pattern_t pattern) {
    if (buzzer_queue != NULL) {
        // Send command to queue (non-blocking)
        xQueueSend(buzzer_queue, &pattern, 0);
    }
}