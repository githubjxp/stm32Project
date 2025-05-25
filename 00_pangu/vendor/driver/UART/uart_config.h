/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-16
 * @version v1.0.0
 **/

#ifndef _UART_CONFIG_H
#define _UART_CONFIG_H

#include "nvic_config.h"

#include "stm32f1xx.h"

typedef struct {
    UART_HandleTypeDef *uart;
    NvicConfig_t nvicConfig;
    uint32_t dmaTxFuncId;
    uint32_t dmaRxFuncId;
    uint32_t uartFuncId;
    uint8_t status;
} UartConfig_t;

typedef enum {
    UART_FUNC_ID_PC_COMMUNICATION = 0,
} UartFuncId_t;

#define UART_RX_PROCESS_BUFFER_SIZE 20
extern uint8_t g_uart1RxProcessBuffer[UART_RX_PROCESS_BUFFER_SIZE];
UartConfig_t *getUartConfigByFuncID(uint32_t funcId);
UART_HandleTypeDef *getUartHandleByFuncID(uint32_t funcId);
void uartInit(void);
#endif // _UART_CONFIG_H