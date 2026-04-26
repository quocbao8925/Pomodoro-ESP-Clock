#ifndef BMI160_H
#define BMI160_H

#include <stdint.h>

void bmi160_init(void);
int16_t bmi160_read_accel_x(void);
int16_t bmi160_read_accel_z(void);

void bmi160_set_accel_lowpower(void);
void bmi160_set_accel_normal(void);

#endif // BMI160_H