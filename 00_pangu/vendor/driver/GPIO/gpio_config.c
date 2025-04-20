/* 
    File: 
    Author: Jiangxiaoping
    Date: 2025-04-14
    Description: 
*/

#include "gpio_config.h"

#include "stm32f1xx.h"


const GpioConfig_t g_gpioTbl[] = {
    {GPIO_ID_GPIOB5, GPIOB, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_LOW, GPIO_FUNC_ID_LED0},
    {GPIO_ID_GPIOE5, GPIOE, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_LOW, GPIO_FUNC_ID_LED1},

    // last idx, invalid idx
    // {GPIO_ID_INVALID, 0, 0, 0, 0, 0, GPIO_FUNC_ID_INVALID},
};

GpioConfig_t *getGpioConfigByFuncID(uint32_t funcId)
{
    for (int i = 0; i < sizeof(g_gpioTbl) / sizeof(g_gpioTbl[0]); i++) {
        if (g_gpioTbl[i].funcId == funcId)
            return &g_gpioTbl[i];
    }

    return NULL;
}

static void enableGpioClk(GPIO_TypeDef *GPIOx)
{
    if (GPIOx == GPIOA)
        __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (GPIOx == GPIOB)
        __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (GPIOx == GPIOC)
        __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (GPIOx == GPIOD)
        __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (GPIOx == GPIOE)
        __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (GPIOx == GPIOF)
        __HAL_RCC_GPIOF_CLK_ENABLE();
    else if (GPIOx == GPIOG)
        __HAL_RCC_GPIOG_CLK_ENABLE();
}

void gpioInit(void)
{
    for (int i = 0; i < sizeof(g_gpioTbl) / sizeof(g_gpioTbl[0]); i++) {
        enableGpioClk(g_gpioTbl[i].gpio);
        GPIO_InitTypeDef gpioInitStruct = {0};
        gpioInitStruct.Pin = g_gpioTbl[i].pin;
        gpioInitStruct.Mode = g_gpioTbl[i].mode;
        gpioInitStruct.Pull = g_gpioTbl[i].pull;
        gpioInitStruct.Speed = g_gpioTbl[i].speed;
        HAL_GPIO_Init(g_gpioTbl[i].gpio, &gpioInitStruct);
    }
}
