
#include "pid.h"

void PID_Init(PID_HandleTypeDef *pid, float kp, float ki, float kd, float max_out, float min_out) {
  pid->kp = kp;
  pid->ki = ki;
  pid->kd = kd;
  pid->max_output = max_out;
  pid->min_output = min_out;
  pid->integral = 0.0f;
  pid->last_error = 0.0f;
}

void PID_SetParam(PID_HandleTypeDef *pid, float kp, float ki, float kd) {
  pid->kp = kp;
  pid->ki = ki;
  pid->kd = kd;
}

float PID_Compute(PID_HandleTypeDef *pid, float error) {
  float output;

  pid->integral += error;

  if (pid->ki != 0.0f) {
    float integral_max = pid->max_output / pid->ki;
    float integral_min = pid->min_output / pid->ki;
    if (pid->integral > integral_max)
      pid->integral = integral_max;
    else if (pid->integral < integral_min)
      pid->integral = integral_min;
  } else {
    pid->integral = 0.0f;
  }

  output = pid->kp * error + pid->ki * pid->integral + pid->kd * (error - pid->last_error);

  if (output > pid->max_output)
    output = pid->max_output;
  else if (output < pid->min_output)
    output = pid->min_output;

  pid->last_error = error;

  return output;
}

void PID_Reset(PID_HandleTypeDef *pid) {
  pid->integral = 0.0f;
  pid->last_error = 0.0f;
}
