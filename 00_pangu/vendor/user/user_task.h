/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-16
 * @version v1.0.0
 */

#ifndef __USER_TASK_H
#define __USER_TASK_H

// led task
#define LED_TASK_STACK_SIZE 128
#define LED_TASK_PRIORITY   1
#define LED_TASK_NAME      "ledTask"

// command task
#define COMMAND_TASK_STACK_SIZE 128
#define COMMAND_TASK_PRIORITY   1
#define COMMAND_TASK_NAME      "commandTask"

void taskEnter(void);

#endif
