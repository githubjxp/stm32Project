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
    GPIO_ID_GPIOA4 = 4,
    GPIO_ID_GPIOA5 = 5,
    GPIO_ID_GPIOA6 = 6,
    GPIO_ID_GPIOA7 = 7,
    GPIO_ID_GPIOA9 = 9,
    GPIO_ID_GPIOA10 = 10,
    GPIO_ID_GPIOB0 = 16,
    GPIO_ID_GPIOB5 = 21,
    GPIO_ID_GPIOB6 = 22,
    GPIO_ID_GPIOB7 = 23,
    GPIO_ID_GPIOB12 = 28,
    GPIO_ID_GPIOB13 = 29,
    GPIO_ID_GPIOB14 = 30,
    GPIO_ID_GPIOB15 = 31,
    GPIO_ID_GPIOC0 = 32,
    GPIO_ID_GPIOD0 = 48,
    GPIO_ID_GPIOE0 = 64,
    GPIO_ID_GPIOE5 = 69,
    GPIO_ID_GPIOF0 = 80,
    GPIO_ID_GPIOG0 = 96,
    GPIO_ID_GPIOG8 = 104,
    GPIO_ID_INVALID = 0xFF,
} GpioId_t;

typedef enum {
    GPIO_FUNC_ID_LED0 = 0,
    GPIO_FUNC_ID_LED1,
    GPIO_FUNC_ID_USART1_TX,
    GPIO_FUNC_ID_USART1_RX,
    GPIO_FUNC_ID_I2C1_SCL,
    GPIO_FUNC_ID_I2C1_SDA,
    GPIO_FUNC_ID_SPI2_CS_FLASH,
    GPIO_FUNC_ID_SPI2_CS_TEST,
    GPIO_FUNC_ID_SPI2_SCK,
    GPIO_FUNC_ID_SPI2_MISO,
    GPIO_FUNC_ID_SPI2_MOSI,
    GPIO_FUNC_ID_INVALID = 0xFF,
} GpioFuncId_t;

GpioConfig_t *getGpioConfigByFuncID(uint32_t funcId);
void gpioInit(void);

#endif
