#ifndef MAIN_H
#define MAIN_H

#include "stm32f4xx_hal.h"

extern I2C_HandleTypeDef hi2c2;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

void MX_GPIO_Init(void);
void MX_I2C2_Init(void);
void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void MX_TIM4_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

#endif
