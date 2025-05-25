/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-16
 * @version v1.0.0
 */

#include "user_task.h"

#include "led.h"
#include "print.h"

#include "stm32f1xx.h"
#include "FreeRTOS.h"
#include "task.h"

void ledTask(void *param)
{
    (void)param;

    while (1) {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5); // Toggle LED0
        PRINT_INFO("LED0 toggled");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void commandTask(void *param)
{
    (void)param;

    while (1) {
        PRINT_INFO("Command task running");
        vTaskDelay(pdMS_TO_TICKS(4000));
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

    ret = xTaskCreate(commandTask, COMMAND_TASK_NAME, COMMAND_TASK_STACK_SIZE, NULL, COMMAND_TASK_PRIORITY, NULL);
    if (ret != pdPASS) {
        PRINT_WARN("commandTask create failed");
    }

    PRINT_INFO("Scheduler started");
    vTaskStartScheduler();
}
