#include "bmi160.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h" 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SW_I2C_SDA GPIO_NUM_5
#define SW_I2C_SCL GPIO_NUM_6
#define BMI160_ADDR 0x69

// Basic bitbanging I2C functions
static void i2c_delay(void) {
    esp_rom_delay_us(5); // ~100kHz
}

static void sw_i2c_start(void) {
    gpio_set_level(SW_I2C_SDA, 1);
    gpio_set_level(SW_I2C_SCL, 1);
    i2c_delay();
    gpio_set_level(SW_I2C_SDA, 0);
    i2c_delay();
    gpio_set_level(SW_I2C_SCL, 0);
}

static void sw_i2c_stop(void) {
    gpio_set_level(SW_I2C_SDA, 0);
    i2c_delay();
    gpio_set_level(SW_I2C_SCL, 1);
    i2c_delay();
    gpio_set_level(SW_I2C_SDA, 1);
    i2c_delay();
}

static bool sw_i2c_write_byte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        gpio_set_level(SW_I2C_SDA, (data & 0x80) ? 1 : 0);
        i2c_delay();
        gpio_set_level(SW_I2C_SCL, 1);
        i2c_delay();
        gpio_set_level(SW_I2C_SCL, 0);
        i2c_delay();
        data <<= 1;
    }
    gpio_set_level(SW_I2C_SDA, 1);
    i2c_delay();
    gpio_set_level(SW_I2C_SCL, 1);
    i2c_delay();
    bool ack = (gpio_get_level(SW_I2C_SDA) == 0);
    gpio_set_level(SW_I2C_SCL, 0);
    i2c_delay();
    return ack;
}

static uint8_t sw_i2c_read_byte(bool ack) {
    uint8_t data = 0;
    gpio_set_direction(SW_I2C_SDA, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(SW_I2C_SDA, 1);

    for (uint8_t i = 0; i < 8; i++) {
        data <<= 1;
        gpio_set_level(SW_I2C_SCL, 1);
        i2c_delay();
        if (gpio_get_level(SW_I2C_SDA)) data |= 1;
        gpio_set_level(SW_I2C_SCL, 0);
        i2c_delay();
    }

    // Send ACK (pull low) or NACK (release high) back to slave.
    gpio_set_level(SW_I2C_SDA, ack ? 0 : 1);
    i2c_delay();
    gpio_set_level(SW_I2C_SCL, 1);
    i2c_delay();
    gpio_set_level(SW_I2C_SCL, 0);
    i2c_delay();
    gpio_set_level(SW_I2C_SDA, 1);

    return data;
}

// Helper: write a single byte to a BMI160 register via CMD.
static void bmi160_write_cmd(uint8_t cmd) {
    sw_i2c_start();
    sw_i2c_write_byte((BMI160_ADDR << 1) | 0);
    sw_i2c_write_byte(0x7E); // CMD register
    sw_i2c_write_byte(cmd);
    sw_i2c_stop();
    vTaskDelay(pdMS_TO_TICKS(5));
}

void bmi160_init(void) {
    gpio_reset_pin(SW_I2C_SDA);
    gpio_set_direction(SW_I2C_SDA, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(SW_I2C_SDA, GPIO_PULLUP_ONLY);

    gpio_reset_pin(SW_I2C_SCL);
    gpio_set_direction(SW_I2C_SCL, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(SW_I2C_SCL, GPIO_PULLUP_ONLY);

    bmi160_write_cmd(0x11); // Accel normal mode (~180µA)
}

void bmi160_set_accel_lowpower(void) {
    bmi160_write_cmd(0x13); // accel_set_pmu_mode: low-power
}

void bmi160_set_accel_normal(void) {
    bmi160_write_cmd(0x11); // accel_set_pmu_mode: normal
}

int16_t bmi160_read_accel_x(void) {
    uint8_t lsb, msb;
    sw_i2c_start();
    sw_i2c_write_byte((BMI160_ADDR << 1) | 0);
    sw_i2c_write_byte(0x12); // ACCEL_X_LSB
    sw_i2c_start();
    sw_i2c_write_byte((BMI160_ADDR << 1) | 1);
    lsb = sw_i2c_read_byte(true);
    msb = sw_i2c_read_byte(false);
    sw_i2c_stop();
    return (int16_t)((msb << 8) | lsb);
}

int16_t bmi160_read_accel_z(void) {
    uint8_t lsb, msb;
    sw_i2c_start();
    sw_i2c_write_byte((BMI160_ADDR << 1) | 0);
    sw_i2c_write_byte(0x16); // ACCEL_Z_LSB
    sw_i2c_start();
    sw_i2c_write_byte((BMI160_ADDR << 1) | 1);
    lsb = sw_i2c_read_byte(true);
    msb = sw_i2c_read_byte(false);
    sw_i2c_stop();
    return (int16_t)((msb << 8) | lsb);
}