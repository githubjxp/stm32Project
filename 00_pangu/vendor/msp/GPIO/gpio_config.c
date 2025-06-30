/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-14
 * @version v1.0.0
 */

#include "gpio_config.h"

#include "stm32f1xx.h"

#include "print.h"

#include <stdbool.h>

const GpioConfig_t g_gpioTbl[] = {
    // UAR1 config
    {GPIO_ID_GPIOA9, GPIOA, GPIO_PIN_9, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_FUNC_ID_USART1_TX},
    {GPIO_ID_GPIOA10, GPIOA, GPIO_PIN_10, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_FUNC_ID_USART1_RX},

    // LED0 config
    {GPIO_ID_GPIOB5, GPIOB, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_LOW, GPIO_FUNC_ID_LED0},

    // I2C1 config
    {GPIO_ID_GPIOB6, GPIOB, GPIO_PIN_6, GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_FUNC_ID_I2C1_SCL}, // I2C1 是低速总线挂载在APB1，所以需要使用GPIO_MODE_AF_OD模式
    {GPIO_ID_GPIOB7, GPIOB, GPIO_PIN_7, GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_FUNC_ID_I2C1_SDA},

    // SPI2 master config
    {GPIO_ID_GPIOB12, GPIOB, GPIO_PIN_12, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_LOW, GPIO_FUNC_ID_SPI2_CS_FLASH},
    {GPIO_ID_GPIOB13, GPIOB, GPIO_PIN_13, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_FUNC_ID_SPI2_SCK},
    {GPIO_ID_GPIOB14, GPIOB, GPIO_PIN_14, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_FUNC_ID_SPI2_MISO},
    {GPIO_ID_GPIOB15, GPIOB, GPIO_PIN_15, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_FUNC_ID_SPI2_MOSI},

    // LED1 config
    {GPIO_ID_GPIOE5, GPIOE, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_LOW, GPIO_FUNC_ID_LED1},

    // SPI2 test cs gpio
    {GPIO_ID_GPIOG8, GPIOG, GPIO_PIN_8, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_LOW, GPIO_FUNC_ID_SPI2_CS_TEST},
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
    bool afio_clk_enable = false;

    for (int i = 0; i < sizeof(g_gpioTbl) / sizeof(g_gpioTbl[0]); i++) {
        enableGpioClk(g_gpioTbl[i].gpio);

        if (!afio_clk_enable && ((g_gpioTbl[i].mode == GPIO_MODE_AF_OD) || (g_gpioTbl[i].mode == GPIO_MODE_AF_PP))) {
            __HAL_RCC_AFIO_CLK_ENABLE(); // Enable AFIO clock
            afio_clk_enable = true;
        }

        GPIO_InitTypeDef gpioInitStruct = {0};
        gpioInitStruct.Pin = g_gpioTbl[i].pin;
        gpioInitStruct.Mode = g_gpioTbl[i].mode;
        gpioInitStruct.Pull = g_gpioTbl[i].pull;
        gpioInitStruct.Speed = g_gpioTbl[i].speed;
        HAL_GPIO_Init(g_gpioTbl[i].gpio, &gpioInitStruct);
    }
}
