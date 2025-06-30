/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-26
 * @version v1.0.0
 **/

#ifndef _UART_IT_H
#define _UART_IT_H

#include "stm32f1xx.h"
#include "uart_config.h"

void uartBuffSwitch(UartConfig_t *uartCfg);

void USART1_IRQHandler(void);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);

#endif // _UART_IT_H