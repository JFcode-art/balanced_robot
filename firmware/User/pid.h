#ifndef PID_H
#define PID_H

#include "main.h"

typedef struct {
  float kp;
  float ki;
  float kd;
  float integral;
  float last_error;
  float max_output;
  float min_output;
} PID_HandleTypeDef;

void PID_Init(PID_HandleTypeDef *pid, float kp, float ki, float kd, float max_out, float min_out);
void PID_SetParam(PID_HandleTypeDef *pid, float kp, float ki, float kd);
float PID_Compute(PID_HandleTypeDef *pid, float error);
void PID_Reset(PID_HandleTypeDef *pid);

#endif
