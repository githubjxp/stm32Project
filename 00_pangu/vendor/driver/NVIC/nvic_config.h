/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-25
 * @version v1.0.0
 * @brief UART
 **/

#ifndef _NVIC_CONFIG_H
#define _NVIC_CONFIG_H

#include "stm32f1xx.h"

typedef struct {
    IRQn_Type irqn;
    uint32_t preemptPriority;
    uint32_t subPriority;
} NvicConfig_t;

#define NVIC_PRIORITY_MIN 0
#define NVIC_PRIORITY_MAX 15

#define NVIC_PRIORITY_UART1 5

#define NVIC_PRIORITY_DMA1_CHANNEL4 5
#define NVIC_PRIORITY_DMA1_CHANNEL5 5



#endif // _DMA_CONFIG_H