#ifndef FILTER_H
#define FILTER_H

#include "main.h"

float Complementary_Filter(float accel_y, float accel_z, float gyro_x);
void Filter_Reset(void);

#endif
