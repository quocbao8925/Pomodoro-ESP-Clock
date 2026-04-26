#include "app_tasks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "u8g2_esp32_hal.h"
#include "esp_wifi.h"
#include "esp_sntp.h"
#include "esp_http_client.h"
#include "esp_event.h"
#include "nvs.h"             
#include "cJSON.h"
#include "globals.h"
#include "bmi160.h"
#include "buzzer.h"
#include "ui.h"
#include "pomodoro.h"
#include "esp_system.h"
#include "driver/usb_serial_jtag.h"


#define BUTTON_PIN GPIO_NUM_10
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1



static EventGroupHandle_t s_wifi_event_group;
static uint8_t s_retry_num = 0;
static uint8_t test = 0;

uint8_t loading_progress = 0; 
bool is_wifi_connected = false;

int8_t current_temp = 0;
uint8_t current_humidity = 0;
uint16_t weather_icon_code = 0;

bool is_serial_mode = false;
uint8_t serial_countdown = 0;

uint8_t sys_volume = 50;              // Default buzzer volume 50%
uint8_t display_timeout_sec = 15;     // Default screen timeout 15s
extern bool is_24h_format;            // Declared in main.c

void serial_listen_task(void *pvParameter) {
    usb_serial_jtag_driver_config_t usb_config = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    usb_serial_jtag_driver_install(&usb_config);

    char rx_buffer[256];
    int rx_pos = 0;
    
    while (1) {
        uint8_t c;
        int len = usb_serial_jtag_read_bytes(&c, 1, pdMS_TO_TICKS(10));
        
        if (len > 0) {
            if (is_serial_mode) {
                if (c == '\n' || c == '\r') {
                    rx_buffer[rx_pos] = '\0';
                    if (rx_pos > 0) {
                        cJSON *root = cJSON_Parse(rx_buffer);
                        if (root) {
                            cJSON *s = cJSON_GetObjectItem(root, "s");
                            cJSON *p = cJSON_GetObjectItem(root, "p");
                            cJSON *t = cJSON_GetObjectItem(root, "t");

                            cJSON *v = cJSON_GetObjectItem(root, "v");   // Volume
                            cJSON *h24 = cJSON_GetObjectItem(root, "h"); // 24h format
                            cJSON *o = cJSON_GetObjectItem(root, "o");   // Screen timeout
                            
                            if (s && p && t) {
                                nvs_handle_t h;
                                if (nvs_open("wifi", NVS_READWRITE, &h) == ESP_OK) {
                                    nvs_set_str(h, "ssid", s->valuestring);
                                    nvs_set_str(h, "pass", p->valuestring);

                                    if (v) nvs_set_u8(h, "vol", (uint8_t)v->valueint);
                                    if (h24) nvs_set_u8(h, "24h", (uint8_t)h24->valueint);
                                    if (o) nvs_set_u8(h, "tout", (uint8_t)o->valueint);
                                    
                                    nvs_commit(h);
                                    nvs_close(h);
                                }
                                
                                struct timeval tv = { .tv_sec = t->valueint, .tv_usec = 0 };
                                settimeofday(&tv, NULL);
                                setenv("TZ", "ICT-7", 1);
                                tzset();
                                
                                is_serial_mode = false;
                                serial_countdown = 0;

                                buzzer_play_pattern(BEEP_TWICE); 
                                vTaskDelay(pdMS_TO_TICKS(1500));
                                esp_restart();
                            }
                            cJSON_Delete(root);
                        }
                        rx_pos = 0;
                    }
                } else if (rx_pos < sizeof(rx_buffer) - 1) {
                    rx_buffer[rx_pos++] = (char)c;
                }
            } else {
                rx_pos = 0;
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}


void parse_weather_json(const char *buffer) {
    cJSON *root = cJSON_Parse(buffer);
    if (!root) return;

    cJSON *main_obj = cJSON_GetObjectItem(root, "main");
    if (main_obj) {
        cJSON *t = cJSON_GetObjectItem(main_obj, "temp");
        cJSON *h = cJSON_GetObjectItem(main_obj, "humidity");
        if (t) current_temp = (int8_t)t->valuedouble;
        if (h) current_humidity = (uint8_t)h->valueint;
    }

    cJSON *weather = cJSON_GetArrayItem(cJSON_GetObjectItem(root, "weather"), 0);
    if (weather) {
        cJSON *id = cJSON_GetObjectItem(weather, "id");
        if (id) weather_icon_code = (uint16_t)id->valueint;
    }
    cJSON_Delete(root); 
}

static void wifi_event_handler(void* arg, esp_event_base_t base, int32_t id, void* data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num++ < 5) esp_wifi_connect();
        else xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

bool connect_wifi_helper(uint32_t timeout_ms) {
    static bool initialized = false;
    if (!initialized) {
        // nvs_flash_init is already handled in app_main for global memory access
        s_wifi_event_group = xEventGroupCreate();
        esp_netif_init();
        esp_event_loop_create_default();

        esp_netif_create_default_wifi_sta();
        // Removed esp_netif_create_default_wifi_ap to save RAM and prioritize energy efficiency

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        esp_wifi_init(&cfg);
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
        initialized = true;
    }

    char ssid[32] = {0}; 
    char pass[64] = {0};
    nvs_handle_t h;
    if (nvs_open("wifi", NVS_READONLY, &h) == ESP_OK) {
        size_t s_len = sizeof(ssid), p_len = sizeof(pass);
        nvs_get_str(h, "ssid", ssid, &s_len);
        nvs_get_str(h, "pass", pass, &p_len);
        nvs_close(h);
    }
    
    if (strlen(ssid) == 0) return false;

    // Clear old event bits before starting a new connection attempt to prevent false returns
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
    
    s_retry_num = 0;

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    
    esp_wifi_set_max_tx_power(78);

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(timeout_ms));
    return (bits & WIFI_CONNECTED_BIT);
}

static int _weather_buf_len = 0; // track fill position

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    if (evt->event_id == HTTP_EVENT_ON_CONNECTED) {
        _weather_buf_len = 0;
    }
    if (evt->event_id == HTTP_EVENT_ON_DATA && evt->user_data) {
        char *buf = (char *)evt->user_data;
        int copy = evt->data_len;
        if (_weather_buf_len + copy > 1023) copy = 1023 - _weather_buf_len;
        if (copy > 0) {
            memcpy(buf + _weather_buf_len, evt->data, copy);
            _weather_buf_len += copy;
            buf[_weather_buf_len] = '\0';
        }
    }
    return ESP_OK;
}
char* http_get_weather(void) {
    char url[256];
    snprintf(url, sizeof(url),
        "http://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
        CONFIG_LAT, CONFIG_LON, CONFIG_WEATHER_API_KEY);

    uint16_t max_buffer = 1024;
    char *buffer = malloc(max_buffer + 1);
    if (!buffer) return NULL;
    buffer[0] = '\0';

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 5000,
        .user_data = buffer,           // pass buffer via user_data
        .event_handler = _http_event_handler,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    if (err == ESP_OK && status == 200 && strlen(buffer) > 0) {
        return buffer;
    }
    free(buffer);
    return NULL;
}

void wifi_sync_task(void *pvParameter) {
    setenv("TZ", "ICT-7", 1);
    tzset();
    static bool sntp_init = false;

    while(1) {
        if (is_serial_mode) { vTaskDelay(pdMS_TO_TICKS(5000)); continue; }
        if (connect_wifi_helper(20000)) {
            is_wifi_connected = true;
            if (!sntp_init) {
                esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
                esp_sntp_setservername(0, "pool.ntp.org");
                esp_sntp_init();
                sntp_init = true;
            }
            
            uint8_t retry = 0;
            while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < 10) {
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            uint8_t api_retries = 0;
            while (api_retries < 2) {
                char *json_data = http_get_weather();
                if (json_data) { 
                    parse_weather_json(json_data); 
                    free(json_data); 
                    break; 
                }
                api_retries++;
                vTaskDelay(pdMS_TO_TICKS(3000)); 
            }
        } else {
            is_wifi_connected = false;
        }
        esp_wifi_disconnect();
        esp_wifi_stop();
        vTaskDelay(pdMS_TO_TICKS(1800 * 1000)); 
    }
}

void wake_up_display() {
    idle_seconds = 0; 
    if (is_display_off) {
        u8g2_SetPowerSave(&u8g2, 0); 
        is_display_off = false;
    }
}

void gyro_task(void *pvParameter) {
    bmi160_init();
    int16_t sleep_z = 0;   
    bool was_off = false; 
    uint16_t load_ms = 0;
    uint8_t target_mode = 0;
    uint8_t last_scr = 0;
    uint8_t sec_tick = 0;

    bmi160_set_accel_lowpower();

    while(1) {
        int16_t az = bmi160_read_accel_z(); 
        int16_t ax = bmi160_read_accel_x();

        if (is_display_off && !was_off) sleep_z = az;
        if (is_display_off && abs(az - sleep_z) > 1000) wake_up_display();
        was_off = is_display_off;

        uint8_t new_scr = current_screen;

        if (az > 8000 ) {
            if (!my_pomo_25.is_running && !my_pomo_25.finish_pomo) { 
                my_pomo_25.minutes = 25; 
                my_pomo_25.seconds = 0; 
            }
            new_scr = 1;
            target_mode = (my_pomo_25.is_running || my_pomo_25.finish_pomo) ? 0 : 25;
            if(!target_mode) load_ms = 0;
        } else if (az < -8000) {
            if (!my_pomo_5.is_running && !my_pomo_5.finish_pomo) { 
                my_pomo_5.minutes = 5; 
                my_pomo_5.seconds = 0; 
            }
            new_scr = 2;
            target_mode = (my_pomo_5.is_running || my_pomo_5.finish_pomo) ? 0 : 5;
            if(!target_mode) load_ms = 0;
        } else {
            if (ax > 8000) { 
                new_scr = 3; 
                if (!is_serial_mode) target_mode = 3;
            } else if (ax < -8000) { 
                new_scr = 0; 
                test = 0; 
                target_mode = 0; 
                load_ms = 0;
            }
        }

        // Auto reset when flipping wrist
        if (last_scr == 1 && new_scr != 1 && my_pomo_25.finish_pomo) pomodoro_reset(&my_pomo_25);
        if (last_scr == 2 && new_scr != 2 && my_pomo_5.finish_pomo) pomodoro_reset(&my_pomo_5);
        if (last_scr != new_scr) {
            load_ms = 0;
            loading_progress = 0;
        }
        
        last_scr = new_scr;
        current_screen = new_scr;

        if (target_mode && !is_display_off) {
            load_ms += 200;
            uint16_t limit = (target_mode == 3) ? 5000 : 3000;
            loading_progress = (target_mode == 3) ? ((load_ms * 100) / 5000) : ((load_ms * 100) / 3000 + 30);
            
            if (load_ms >= limit) {
                if (target_mode == 3) {
                    is_serial_mode = true;
                    serial_countdown = 45; 
                    buzzer_play_pattern(BEEP_LONG);
                } else {
                    pomodoro_start_new(target_mode == 25 ? &my_pomo_25 : &my_pomo_5, target_mode);
                    pomodoro_reset(target_mode == 25 ? &my_pomo_5 : &my_pomo_25); 
                    buzzer_play_pattern(BEEP_SHORT);
                }
                target_mode = 0; load_ms = 0; loading_progress = 0;
            }
        } else {
            loading_progress = 0; 
        }

        if (is_serial_mode) {
            if (++sec_tick >= 5) {
                sec_tick = 0;
                if (serial_countdown > 0) serial_countdown--;
                if (serial_countdown == 0) {
                    is_serial_mode = false;
                    buzzer_play_pattern(BEEP_SHORT);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

uint8_t get_weather_icon_hex(uint16_t weather_code, bool is_night) {
    if (weather_code >= 200 && weather_code < 700) return 0x43; 
    else if (weather_code >= 700 && weather_code < 800) return 0x40; 
    else if (weather_code == 800) return is_night ? 0x42 : 0x45;          
    else if (weather_code == 801 || weather_code == 802) return is_night ? 0x42 : 0x41;          
    else if (weather_code == 803 || weather_code == 804) return is_night ? 0x42 : 0x40; 
    
    return 0x45; 
}

// Display interface
void display_task(void *pvParameter) {
    
    nvs_handle_t h;
    if (nvs_open("wifi", NVS_READONLY, &h) == ESP_OK) {
        nvs_get_u8(h, "vol", &sys_volume);
        
        uint8_t h24_val = 0;
        if (nvs_get_u8(h, "24h", &h24_val) == ESP_OK) {
            is_24h_format = (h24_val == 1);
        }
        
        nvs_get_u8(h, "tout", &display_timeout_sec);
        nvs_close(h);
    }

    if (sys_volume > 100) sys_volume = 50; 
    if (display_timeout_sec < 5 || display_timeout_sec > 60) display_timeout_sec = 15;

    TickType_t last_tick = xTaskGetTickCount();

    while(1) {

        TickType_t now = xTaskGetTickCount();
        

        if (now - last_tick >= pdMS_TO_TICKS(1000)) {

            if (current_screen == 3) {
                idle_seconds = 0; 
            } else { 
                idle_seconds++; 
            }
            last_tick = now; 
        }

        if (idle_seconds > display_timeout_sec && !is_display_off) {
            u8g2_SetPowerSave(&u8g2, 1); 
            is_display_off = true;
            bmi160_set_accel_lowpower(); // Put gyro in low-power mode when display sleeps
        }

        time_t now_time; 
        struct tm ti;
        time(&now_time); 
        localtime_r(&now_time, &ti);
        uint8_t h = ti.tm_hour;
        uint8_t m = ti.tm_min;
        bool is_pm = false;
        
        bool is_night = (h >= 18 || h < 6);

        if (!is_24h_format) {
            is_pm = (h >= 12);
            h = (h > 12) ? h - 12 : (h == 0 ? 12 : h);
        }

        if (!is_display_off) {
            if (current_screen == 0) {
                squix_ui_t sq = {0}; 
                snprintf(sq.time, sizeof(sq.time), "%02d:%02d", h, m);
                strftime(sq.date, sizeof(sq.date), "%d %b", &ti);
                sq.is_24h = is_24h_format;
                sq.is_pm = is_pm;

                if (is_wifi_connected) {
                    strncpy(sq.city, "HCMC", sizeof(sq.city));
                    sq.temp = current_temp; 
                    sq.humidity = current_humidity; 
                    sq.icon = get_weather_icon_hex(weather_icon_code, is_night);
                } else {
                    strncpy(sq.city, "OFFLINE", sizeof(sq.city));
                    sq.icon = 0;
                }
                draw_classic_squix(&u8g2, sq, my_pomo_5.is_running || my_pomo_25.is_running, is_serial_mode);

            } else if (current_screen == 1) {
                draw_pomodoro_vertical(&u8g2, &my_pomo_25, true, loading_progress);
            } else if (current_screen == 2) {
                draw_pomodoro_vertical(&u8g2, &my_pomo_5, false, loading_progress);
            } else if (current_screen == 3) {
                draw_wifi_config_ui(&u8g2, is_serial_mode, serial_countdown, loading_progress);
            } else if (current_screen == 4) {
                u8g2_ClearBuffer(&u8g2); 
                u8g2_SendBuffer(&u8g2);
            }
        }
        
        if (my_pomo_25.flash_timer > 0) { 
            wake_up_display(); 
            my_pomo_25.flash_timer--; 
        }
        if (my_pomo_5.flash_timer > 0) { 
            wake_up_display(); 
            my_pomo_5.flash_timer--; 
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}