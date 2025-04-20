/* 
    File: 
    Author: Jiangxiaoping
    Date: 2025-04-16
    Description: 
*/

#ifndef _LED_H
#define _LED_H

#include "gpio_config.h"

#include "stdint.h"

typedef enum {
    LED_STATE_OFF = 0,
    LED_STATE_ON,
    LED_STATE_ALAWAYS_ON = LED_STATE_ON,
    // LED_STATE_TOGGLE,
} LedState_t;

typedef enum {
    LED_INVALID_LEVEL_HIGH = 0,
    LED_INVALID_LEVEL_LOW,
} LedBlinkState_t;

#define LED0 0
#define LED1 1
#define LED_MAX 2

typedef struct {
    uint32_t gpio_func_id;
    GpioConfig_t *gpioConf;
    int8_t state;
    uint32_t invalidLevel;
} Led_t;

#endif
