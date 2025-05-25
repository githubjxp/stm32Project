/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-14
 * @version v1.0.0
 */

#ifndef _GPIO_CONFIG_H
#define _GPIO_CONFIG_H

#include "stm32f1xx.h"

typedef struct {
    uint32_t       id; // GPIOx * 16 + GPIO_PIN_y = GPIOB + GPIO_PIN_5 = 1 * 16 + 5 = 21
    GPIO_TypeDef*  gpio;
    uint32_t       pin;
    uint32_t       mode;
    uint32_t       pull;
    uint32_t       speed;
    uint32_t       funcId;
} GpioConfig_t;

typedef enum {
    GPIO_ID_GPIOA0 = 0,
    GPIO_ID_GPIOA9 = 9,
    GPIO_ID_GPIOA10 = 10,
    GPIO_ID_GPIOB0 = 16,
    GPIO_ID_GPIOB5 = 21,
    GPIO_ID_GPIOC0 = 32,
    GPIO_ID_GPIOD0 = 48,
    GPIO_ID_GPIOE0 = 64,
    GPIO_ID_GPIOE5 = 69,
    GPIO_ID_GPIOF0 = 80,
    GPIO_ID_GPIOG0 = 96,
    GPIO_ID_INVALID = 0xFF,
} GpioId_t;

typedef enum {
    GPIO_FUNC_ID_LED0 = 0,
    GPIO_FUNC_ID_LED1,
    GPIO_FUNC_ID_USART1_TX,
    GPIO_FUNC_ID_USART1_RX,
    GPIO_FUNC_ID_INVALID = 0xFF,
} GpioFuncId_t;

GpioConfig_t *getGpioConfigByFuncID(uint32_t funcId);
void gpioInit(void);

#endif
