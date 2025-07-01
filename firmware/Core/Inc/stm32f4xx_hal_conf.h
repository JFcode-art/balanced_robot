
#ifndef STM32F4XX_HAL_CONF_H
#define STM32F4XX_HAL_CONF_H

#define HAL_USE_RCC                  1
#define HAL_USE_GPIO                 1
#define HAL_USE_I2C                  1
#define HAL_USE_UART                 1
#define HAL_USE_TIM                  1
#define HAL_USE_IWDG                 1

#define HSE_VALUE                    8000000U
#define HSI_VALUE                    16000000U
#define LSE_VALUE                    32768U
#define LSI_VALUE                    32000U

#define PREFETCH_ENABLE              1
#define INSTRUCTION_CACHE_ENABLE     1
#define DATA_CACHE_ENABLE            1

#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_iwdg.h"
#include "stm32f4xx_hal.h"

#endif
