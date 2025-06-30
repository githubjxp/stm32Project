/**
 * @file hw_interface.h
 * @author jiangxiaoping
 * @date 2025-07-07
 * @version v1.0.0
 */

#include "hw_interface.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f1xx_hal.h"

void delayMs(uint32_t ms) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        vTaskDelay(pdMS_TO_TICKS(ms));
    } else {
        HAL_Delay(ms);
    }
}
