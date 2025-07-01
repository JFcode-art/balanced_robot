#include "filter.h"
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"

#define COMPLEMENTARY_ALPHA 0.98f

static float angle = 0.0f;
static TickType_t last_tick = 0;

float Complementary_Filter(float accel_y, float accel_z, float gyro_x) {
  float angle_accel;
  TickType_t now = xTaskGetTickCount();
  float dt = (last_tick == 0) ? 0.001f : (now - last_tick) / 1000.0f;
  last_tick = now;

  if (dt > 0.01f) dt = 0.01f;

  angle_accel = atan2f(accel_y, accel_z) * 57.3f;

  if (isnan(angle_accel)) angle_accel = 0.0f;

  angle = COMPLEMENTARY_ALPHA * (angle + gyro_x * dt) + (1 - COMPLEMENTARY_ALPHA) * angle_accel;

  return angle;
}

void Filter_Reset(void) {
  angle = 0.0f;
  last_tick = 0;
}
