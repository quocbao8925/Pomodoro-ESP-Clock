#include "bmi160.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h" // Dùng hàm esp_rom_delay_us()
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Định nghĩa 2 chân Bitbanging mới
#define SW_I2C_SDA GPIO_NUM_5
#define SW_I2C_SCL GPIO_NUM_6
#define BMI160_ADDR 0x69

// ==========================================
// CÁC HÀM BITBANGING I2C CƠ BẢN
// ==========================================
static void i2c_delay(void) {
    esp_rom_delay_us(5); // Delay 5 micro-giây (~100kHz Standard Mode)
}

static void sw_i2c_start(void) {
    gpio_set_level(SW_I2C_SDA, 1);
    gpio_set_level(SW_I2C_SCL, 1);
    i2c_delay();
    gpio_set_level(SW_I2C_SDA, 0); // SDA kéo xuống trước SCL -> Lệnh START
    i2c_delay();
    gpio_set_level(SW_I2C_SCL, 0);
}

static void sw_i2c_stop(void) {
    gpio_set_level(SW_I2C_SDA, 0);
    i2c_delay();
    gpio_set_level(SW_I2C_SCL, 1);
    i2c_delay();
    gpio_set_level(SW_I2C_SDA, 1); // SDA đẩy lên sau SCL -> Lệnh STOP
    i2c_delay();
}

// Bạn sẽ cần tự viết/sưu tầm thêm 2 hàm này:
static bool sw_i2c_write_byte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        gpio_set_level(SW_I2C_SDA, (data & 0x80) ? 1 : 0);
        i2c_delay();
        gpio_set_level(SW_I2C_SCL, 1);
        i2c_delay();
        gpio_set_level(SW_I2C_SCL, 0);
        i2c_delay();
        data <<= 1;
    }
    // Nhả SDA để nhận ACK từ slave
    gpio_set_level(SW_I2C_SDA, 1);
    i2c_delay();
    gpio_set_level(SW_I2C_SCL, 1);
    i2c_delay();
    bool ack = (gpio_get_level(SW_I2C_SDA) == 0); // ACK = 0
    gpio_set_level(SW_I2C_SCL, 0);
    i2c_delay();
    return ack;
}
static uint8_t sw_i2c_read_byte(bool ack) {
    uint8_t data = 0;
    // Set SDA as input to read data
    gpio_set_direction(SW_I2C_SDA, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(SW_I2C_SDA, 1); // Release SDA

    for (int i = 0; i < 8; i++) {
        data <<= 1;
        gpio_set_level(SW_I2C_SCL, 1);
        i2c_delay();
        if (gpio_get_level(SW_I2C_SDA)) {
            data |= 1;
        }
        gpio_set_level(SW_I2C_SCL, 0);
        i2c_delay();
    }

    // Set SDA as output to send ACK/NACK
    gpio_set_direction(SW_I2C_SDA, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(SW_I2C_SDA, ack ? 0 : 1);
    i2c_delay();
    gpio_set_level(SW_I2C_SCL, 1);
    i2c_delay();
    gpio_set_level(SW_I2C_SCL, 0);
    i2c_delay();
    gpio_set_level(SW_I2C_SDA, 1); // Release SDA

    return data;
}



// ==========================================
// VIẾT LẠI CÁC HÀM DRIVER BMI160
// ==========================================
void bmi160_init(void) {
    // 1. Cấu hình chân Open-Drain kèm điện trở kéo lên
    gpio_reset_pin(SW_I2C_SDA);
    gpio_set_direction(SW_I2C_SDA, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(SW_I2C_SDA, GPIO_PULLUP_ONLY);

    gpio_reset_pin(SW_I2C_SCL);
    gpio_set_direction(SW_I2C_SCL, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(SW_I2C_SCL, GPIO_PULLUP_ONLY);

    // Bắt đầu nhịp I2C để đánh thức BMI160
    sw_i2c_start();
    sw_i2c_write_byte((BMI160_ADDR << 1) | 0); // Ghi
    sw_i2c_write_byte(0x7E);                   // Thanh ghi CMD
    sw_i2c_write_byte(0x11);                   // Lệnh bật
    sw_i2c_stop();

    vTaskDelay(pdMS_TO_TICKS(5)); 
}

int16_t bmi160_read_accel_y(void) {
    uint8_t lsb, msb;
    
    // Gửi địa chỉ thanh ghi cần đọc
    sw_i2c_start();
    sw_i2c_write_byte((BMI160_ADDR << 1) | 0);
    sw_i2c_write_byte(0x14); // ACCEL_Y_LSB
    
    // Khởi động lại (Repeated Start) để chuyển sang chế độ đọc
    sw_i2c_start();
    sw_i2c_write_byte((BMI160_ADDR << 1) | 1);
    
    // Đọc byte đầu (Gửi tín hiệu ACK để đòi byte tiếp theo)
    lsb = sw_i2c_read_byte(true); 
    // Đọc byte cuối (Gửi tín hiệu NACK để báo kết thúc)
    msb = sw_i2c_read_byte(false); 
    
    sw_i2c_stop();
    
    return (int16_t)((msb << 8) | lsb); 
}
int16_t bmi160_read_accel_z(void) {
    uint8_t lsb, msb;
    
    // Gửi địa chỉ thanh ghi Z cần đọc
    sw_i2c_start();
    sw_i2c_write_byte((BMI160_ADDR << 1) | 0);
    sw_i2c_write_byte(0x16); // ACCEL_Z_LSB (Khác biệt duy nhất ở đây)
    
    // Khởi động lại (Repeated Start) để chuyển sang chế độ đọc
    sw_i2c_start();
    sw_i2c_write_byte((BMI160_ADDR << 1) | 1);
    
    // Đọc byte đầu (LSB) và gửi ACK
    lsb = sw_i2c_read_byte(true); 
    // Đọc byte cuối (MSB) và gửi NACK
    msb = sw_i2c_read_byte(false); 
    
    sw_i2c_stop();
    
    return (int16_t)((msb << 8) | lsb); 
}