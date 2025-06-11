/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-16
 * @version v1.0.0
 */

#ifndef __USER_TASK_H
#define __USER_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// led task
#define LED_TASK_STACK_SIZE 128
#define LED_TASK_PRIORITY   1
#define LED_TASK_NAME      "ledTask"

// command task
#define CMD_TASK_STACK_SIZE 128
#define CMD_TASK_PRIORITY   1
#define CMD_TASK_NAME      "commandTask"

extern TaskHandle_t g_cmdTaskHandle;
extern TaskHandle_t g_ledTaskHandle;

void taskEnter(void);

#endif
