
#include "gpio.h"

void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* PA2(USART2_TX), PA3(USART2_RX) - configured by HAL_UART_MspInit */
  /* PA6(TIM3_CH1), PA7(TIM3_CH2)   - configured by HAL_TIM_PWM_MspInit */
  /* PB0(TIM3_CH3), PB1(TIM3_CH4)   - configured by HAL_TIM_PWM_MspInit */
  /* PB10(I2C2_SCL), PB11(I2C2_SDA) - configured by HAL_I2C_MspInit */

  /* Additional GPIO outputs can be added here */
}
