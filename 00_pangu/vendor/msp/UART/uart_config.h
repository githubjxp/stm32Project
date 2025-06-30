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
    char *buff;
    const uint32_t size;
    uint32_t pos;
} UartBuff_t;

enum {
    UART_BUFF_STATE_BUFF0_RECEIVING = 0,
    UART_BUFF_STATE_BUFF1_RECEIVING,

    UART_BUFF_STATE_UNUSED,
};

typedef struct {
    UART_HandleTypeDef *uart;
    NvicConfig_t nvicConfig;
    uint32_t dmaTxFuncId;
    uint32_t dmaRxFuncId;
    uint32_t uartFuncId;
    uint8_t status;
    UartBuff_t rxBuff0;
    UartBuff_t rxBuff1;
    uint8_t rxBuffState;
} UartConfig_t;

typedef enum {
    UART_FUNC_ID_PC_COMMUNICATION = 0,
} UartFuncId_t;

#define UART_RX_BUFF_SIZE 64
extern uint8_t g_uart1RxBuff1[UART_RX_BUFF_SIZE];
UartConfig_t *getUartConfigByFuncID(uint32_t funcId);
UART_HandleTypeDef *getUartHandleByFuncID(uint32_t funcId);
void uartInit(void);
#endif // _UART_CONFIG_H