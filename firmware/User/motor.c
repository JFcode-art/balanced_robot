
#include "motor.h"

#define MOTOR_PWM_MAX 999

void Motor_Init(void) {
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

  Motor_SetSpeed(MOTOR_LEFT, 0);
  Motor_SetSpeed(MOTOR_RIGHT, 0);
}

void Motor_SetSpeed(Motor_TypeDef motor, int16_t speed) {
  if (speed > MOTOR_PWM_MAX) speed = MOTOR_PWM_MAX;
  if (speed < -MOTOR_PWM_MAX) speed = -MOTOR_PWM_MAX;

  if (motor == MOTOR_LEFT) {
    if (speed >= 0) {
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, speed);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
    } else {
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, -speed);
    }
  } else {
    if (speed >= 0) {
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, speed);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
    } else {
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, -speed);
    }
  }
}

void Encoder_Init(void) {
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

  __HAL_TIM_SET_COUNTER(&htim2, 0);
  __HAL_TIM_SET_COUNTER(&htim4, 0);
}
