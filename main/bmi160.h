#ifndef BMI160_H
#define BMI160_H

#include <stdint.h>

void bmi160_init(void);
int16_t bmi160_read_accel_y(void);
int16_t bmi160_read_accel_z(void);

#endif // BMI160_H