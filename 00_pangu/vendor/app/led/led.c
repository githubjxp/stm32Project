/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-16
 * @version v1.0.0
 */

#include "led.h"

#include "stm32f1xx.h"

#include "app.h"
#include "gpio_config.h"
#include "print.h"
#include "tag.h"

static Led_t g_ledTbl[LED_MAX] = {
    {
        .gpio_func_id = GPIO_FUNC_ID_LED0,
        .gpioConf = NULL,
        .state = LED_STATE_OFF,
        .invalidLevel = LED_INVALID_LEVEL_LOW,
    },
    {
        .gpio_func_id = GPIO_FUNC_ID_LED1,
        .gpioConf = NULL,
        .state = LED_STATE_OFF,
        .invalidLevel = LED_INVALID_LEVEL_LOW,
    }
};

static void ledEnable(Led_t *led, uint8_t state)
{
    if (led == NULL || led->gpioConf == NULL) {
        PRINT_ERROR("led is NULL or led->gpioConf is NULL!");
        return;
    }

    uint8_t pinState;
    if (state == LED_STATE_OFF) {
        pinState = GPIO_PIN_RESET;
    } else if (state == LED_STATE_ALAWAYS_ON) {
        pinState = GPIO_PIN_SET;
    } else {
        PRINT_ERROR("Invalid state: %d", state);
        return;
    }

    if (led->invalidLevel == LED_INVALID_LEVEL_LOW) {
        pinState = (pinState + 1) % 2; // 2: Invert the pin state
    }

    HAL_GPIO_WritePin(led->gpioConf->gpio, led->gpioConf->pin, pinState);
    led->state = state;
}

static int32_t ledOpen(App_t *app)
{
    return SUCCESS;
}

static int32_t ledClose(App_t *app)
{
    return SUCCESS;
}

static int32_t ledConfig(App_t *app, void *config)
{
    // Configure LED GPIO pins here if needed
    return SUCCESS;
}

static int32_t ledProcessData(App_t *app, uint32_t tag, uint8_t *data, uint32_t len)
{
    // Process LED data here if needed
    return SUCCESS;
}

static int32_t ledInit(App_t *app)
{
    if (app == NULL) {
        PRINT_ERROR("app is NULL!");
        return ERROR;
    }

    for (int i = 0; i < LED_MAX; i++) {
        g_ledTbl[i].gpioConf = getGpioConfigByFuncID(g_ledTbl[i].gpio_func_id);
        if (g_ledTbl[i].gpioConf == NULL) {
            PRINT_ERROR("g_ledTbl[%d].gpioConf is NULL!", i);
            return ERROR;
        }
    }

    app->priv = &g_ledTbl;

    ledEnable(&g_ledTbl[LED0], LED_STATE_ON);
    ledEnable(&g_ledTbl[LED1], LED_STATE_ON);

    return SUCCESS;
}

static AppOps_t ledOps = {
    .init = ledInit,
    .open = ledOpen,
    .close = ledClose,
    .processData = ledProcessData,
    .config = ledConfig,
};

static App_t ledApp = {
    .tag = APP_TAG_LED,
    .appData = NULL,
    .appDataLen = 0,
    .priv = NULL,
    .ops = &ledOps,
};

void ledAppRegister(void)
{
    appRegister(&ledApp);
    PRINT_INFO("ledAppRegister done!");
}

APP_INIT(ledAppRegister);
