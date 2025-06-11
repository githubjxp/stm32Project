/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-16
 * @version v1.0.0
 */

#include "user_task.h"

#include "led.h"
#include "print.h"
#include "uart_config.h"

#include "stm32f1xx.h"
#include "FreeRTOS.h"
#include "task.h"

TaskHandle_t g_ledTaskHandle = NULL;
TaskHandle_t g_cmdTaskHandle = NULL;

void ledTask(void *param)
{
    (void)param;

    while (1) {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5); // Toggle LED0
        PRINT_INFO("LED0 toggled");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static uint8_t *getUartProcessData(UartBuff_t *uartBuff)
{
    uint8_t *data = (uint8_t *)pvPortMalloc(uartBuff->size);
    if (data == NULL) {
        PRINT_ERROR("Failed to allocate memory for UART data processing");
        return NULL;
    }
    memset(data, 0, uartBuff->size);
    memcpy(data, uartBuff->buff, uartBuff->pos);
    data[uartBuff->pos] = '\0';
    uartBuff->pos = 0;

    return data;
}

void cmdTask(void *param)
{
    (void)param;

    while (1) {
        PRINT_INFO("Command task running");
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Wait for notification
        vTaskDelay(pdMS_TO_TICKS(1000));
        UartConfig_t *uartCfg = getUartConfigByFuncID(UART_FUNC_ID_PC_COMMUNICATION);
        uint8_t *uartData = NULL;
        PRINT_INFO("buff size: %d %d, buff0 pos: %d, buff1 pos: %d, state: %d", uartCfg->rxBuff0.size,
            uartCfg->rxBuff1.size, uartCfg->rxBuff0.pos, uartCfg->rxBuff1.pos, uartCfg->rxBuffState);
        if (uartCfg->rxBuffState == UART_BUFF_STATE_BUFF0_RECEIVING) {
            uartData = getUartProcessData(&uartCfg->rxBuff1); // 0 recv 1 process
        } else if (uartCfg->rxBuffState == UART_BUFF_STATE_BUFF1_RECEIVING) {
            uartData = getUartProcessData(&uartCfg->rxBuff0); // 1 recv 0 process
        }
        
        PRINT_INFO("Received data: %s", uartData);
        vPortFree(uartData);
    }
}

void taskEnter(void)
{
    PRINT_INFO("taskEnter");
    int32_t ret;

    ret = xTaskCreate(ledTask, LED_TASK_NAME, LED_TASK_STACK_SIZE, NULL, LED_TASK_PRIORITY, NULL);
    if (ret != pdPASS) {
        PRINT_WARN("ledTask create failed");
    }

    ret = xTaskCreate(cmdTask, CMD_TASK_NAME, CMD_TASK_STACK_SIZE, NULL, CMD_TASK_PRIORITY, &g_cmdTaskHandle);
    if (ret != pdPASS) {
        PRINT_WARN("cmdTask create failed");
    }

    PRINT_INFO("Scheduler started");
    vTaskStartScheduler();
}
