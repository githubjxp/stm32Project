/* 
    File: 
    Author: Jiangxiaoping
    Date: 2025-04-09
    Description: 
*/

#ifndef __USERCONFIGHOOK_H
#define __USERCONFIGHOOK_H

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                        char * pcTaskName);
#endif

#if (configUSE_TICK_HOOK == 1)
void vApplicationTickHook(void);
#endif


#endif