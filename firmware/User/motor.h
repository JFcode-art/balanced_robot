#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"

typedef enum {
  MOTOR_LEFT = 0,
  MOTOR_RIGHT = 1
} Motor_TypeDef;

void Motor_Init(void);
void Motor_SetSpeed(Motor_TypeDef motor, int16_t speed);
void Encoder_Init(void);

#endif
