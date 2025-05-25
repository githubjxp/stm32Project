/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-26
 * @version v1.0.0
 **/

#include "uart_it.h"
#include "uart_config.h"
#include "dma_config.h"
#include "print.h"

#include "led.h"
#include "stm32f1xx.h"

#include "string.h"

void USART1_IRQHandler(void)
{
    UART_HandleTypeDef *uart = getUartHandleByFuncID(UART_FUNC_ID_PC_COMMUNICATION);
    HAL_UART_IRQHandler(uart);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
    PRINT_FOR_TEST("noting to do");
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
    ledTest();
    PRINT_FOR_TEST("noting to do");        
}

/**
 * callback for HAL_UART_Receive_IT or HAL_UART_Receive_DMA
 * called when the reception is complete
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        PRINT_FOR_TEST("sr = 0x%x", huart->Instance->DR);
        g_uart1RxProcessBuffer[UART_RX_PROCESS_BUFFER_SIZE - 1] = 0x77; // Null-terminate the string
        if (huart->hdmarx != NULL) {
            HAL_UART_Receive_DMA(huart, g_uart1RxProcessBuffer, UART_RX_PROCESS_BUFFER_SIZE);
        } else {
            HAL_UART_Receive_IT(huart, g_uart1RxProcessBuffer, UART_RX_PROCESS_BUFFER_SIZE);
        }
    }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
    PRINT_FOR_TEST("noting to do");
}

/**
 * callback for HAL_UARTEx_ReceiveToIdle_IT or HAL_UARTEx_ReceiveToIdle_DMA
 * called when the reception is complete or idle 
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (huart->Instance == USART1) {
        PRINT_FOR_TEST("sr = 0x%x", huart->Instance->DR);
        g_uart1RxProcessBuffer[UART_RX_PROCESS_BUFFER_SIZE - 1] = 0x77; // Null-terminate the string
        HAL_UARTEx_ReceiveToIdle_IT(huart, g_uart1RxProcessBuffer, UART_RX_PROCESS_BUFFER_SIZE);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    PRINT_ERROR("UART error: %d", huart->ErrorCode);
}
