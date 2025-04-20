/* 
    File: 
    Author: Jiangxiaoping
    Date: 2025-04-09
    Description: 
*/

#include "FreeRTOS.h"
#include "task.h"

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                        char * pcTaskName)
{
    return;
}
#endif

