#ifndef APP_TASKS_H
#define APP_TASKS_H


void gyro_task(void *pvParameter);
void button_task(void *pvParameter);
void display_task(void *pvParameter);
void wifi_sync_task(void *pvParameter);
void serial_listen_task(void *pvParameter);

#endif // APP_TASKS_H