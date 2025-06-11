/* 
    File: 
    Author: Jiangxiaoping
    Date: 2025-04-09
    Description: 
*/

#include "UserConfigHook.h"

#include "stm32f1xx.h"

#include "FreeRTOS.h"
#include "task.h"

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                        char * pcTaskName)
{
    return;
}
#endif

#if (configUSE_TICK_HOOK == 1)
void vApplicationTickHook(void)
{
    // HAL_IncTick();  // 在FreeRTOS的Tick钩子中更新HAL计时
}
#endif
