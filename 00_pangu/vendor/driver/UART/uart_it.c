/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-26
 * @version v1.0.0
 **/

#include "uart_it.h"

#include "dma_config.h"
#include "print.h"
#include "uart_config.h"
#include "user_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f1xx.h"

#include "string.h"

typedef enum {
    IRQ_TYPE_IDLE = 0,
    IRQ_TYPE_RECEIVE_COMPLETE,
    IRQ_TYPE_RECEIVE_HALF_COMPLETE,
    IRQ_TYPE_TRANSMIT_COMPLETE,
    IRQ_TYPE_TRANSMIT_HALF_COMPLETE,
} IrqType_t;

static void getCurrBuffSize(UartConfig_t *uartCfg) {
    // IT mode
    if (uartCfg->uart->hdmarx == NULL) {
        if (uartCfg->rxBuffState == UART_BUFF_STATE_BUFF0_RECEIVING) {
            uartCfg->rxBuff0.pos = uartCfg->uart->RxXferCount;
        } else if (uartCfg->rxBuffState == UART_BUFF_STATE_BUFF1_RECEIVING) {
            uartCfg->rxBuff1.pos = uartCfg->uart->RxXferCount;
        }
    }

    // DMA mode
    if (uartCfg->uart->hdmarx != NULL) {
        if (uartCfg->rxBuffState == UART_BUFF_STATE_BUFF0_RECEIVING) {
            uartCfg->rxBuff0.pos = uartCfg->rxBuff0.size - __HAL_DMA_GET_COUNTER(uartCfg->uart->hdmarx);
        } else if (uartCfg->rxBuffState == UART_BUFF_STATE_BUFF1_RECEIVING) {
            uartCfg->rxBuff1.pos = uartCfg->rxBuff1.size - __HAL_DMA_GET_COUNTER(uartCfg->uart->hdmarx);
        }
    }
}

static void uartReceiveToIdle(UartConfig_t *uartCfg, UartBuff_t *uartBuff)
{
    if (uartCfg->uart->hdmarx != NULL) {
        HAL_UARTEx_ReceiveToIdle_DMA(uartCfg->uart, uartBuff->buff + uartBuff->pos, uartBuff->size - uartBuff->pos);
    } else {
        HAL_UART_Receive_IT(uartCfg->uart, uartBuff->buff + uartBuff->pos, uartBuff->size - uartBuff->pos);
    }
}

/*
 * 切换uart的buff，状态如下
 * 触发中断时buff状态	当前接收buff	中断类型	下次接收buff	处理buff
 * buff0空buff1空	      NA	         NA	          0	            NA
 * buff0空buff1余	      1	             IDLE	      0	            1
 * buff0空buff1满	      1	             COMPLETE	  0	            1
 * buff0余buff1空	      0	             IDLE	      1	            0
 * buff0余buff1余	      0	             IDLE	      0	            1
 * buff0余buff1余	      1	             IDLE	      1	            0
 * buff0余buff1满	      0	             IDLE	      0	            1
 * buff0余buff1满	      1	             COMPLETE	  0+数据溢出错误 1
 * buff0满buff1空	      0	             COMPLETE	  1	            0
 * buff0满buff1余	      0	             COMPLETE	  1+数据溢出错误 0
 * buff0满buff1余	      1	             IDLE	      1         	0
 * buff0满buff1满	      0	             COMPLETE	  1+数据溢出错误 0
 * buff0满buff1满	      1	             COMPLETE	  0+数据溢出错误 0
 * Ps. 发生数据溢出问题，丢弃溢出的数据，其他数据正常处理
 */
static void uartBuffSwitch(UartConfig_t *uartCfg)
{
    getCurrBuffSize(uartCfg);
    PRINT_FOR_TEST("buff0 pos: %d, buff1 pos: %d, state: %d",
        uartCfg->rxBuff0.pos, uartCfg->rxBuff1.pos, uartCfg->rxBuffState);

    // Switch buff without error: other buff is empty
    if ((uartCfg->rxBuffState == UART_BUFF_STATE_BUFF0_RECEIVING) && (uartCfg->rxBuff1.pos == 0)) {
        uartReceiveToIdle(uartCfg, &uartCfg->rxBuff1);
        uartCfg->rxBuffState = UART_BUFF_STATE_BUFF1_RECEIVING;
    } else if ((uartCfg->rxBuffState == UART_BUFF_STATE_BUFF1_RECEIVING) && (uartCfg->rxBuff0.pos == 0)) {
        uartReceiveToIdle(uartCfg, &uartCfg->rxBuff0);
        uartCfg->rxBuffState = UART_BUFF_STATE_BUFF0_RECEIVING;
    }

    // not switch buff: current buff is not full and other buff is not empty
    else if ((uartCfg->rxBuffState == UART_BUFF_STATE_BUFF0_RECEIVING) &&
        (uartCfg->rxBuff0.pos < uartCfg->rxBuff0.pos) && (uartCfg->rxBuff1.pos > 0)) {
        uartReceiveToIdle(uartCfg, &uartCfg->rxBuff0);
        uartCfg->rxBuffState = UART_BUFF_STATE_BUFF0_RECEIVING;
    } else if ((uartCfg->rxBuffState == UART_BUFF_STATE_BUFF1_RECEIVING) &&
        (uartCfg->rxBuff1.pos < uartCfg->rxBuff1.pos) && (uartCfg->rxBuff0.pos > 0)) {
        uartReceiveToIdle(uartCfg, &uartCfg->rxBuff1);
        uartCfg->rxBuffState = UART_BUFF_STATE_BUFF1_RECEIVING;
    }

    // Switch buff with warn: current buff is full and other buff is not empty
    else if ((uartCfg->rxBuffState == UART_BUFF_STATE_BUFF0_RECEIVING) &&
        (uartCfg->rxBuff0.pos >= uartCfg->rxBuff0.size) && (uartCfg->rxBuff1.pos > 0)) {
        uartReceiveToIdle(uartCfg, &uartCfg->rxBuff1);
        uartCfg->rxBuffState = UART_BUFF_STATE_BUFF1_RECEIVING;
        PRINT_WARN("current buff0 is full, but other buff1 is not empty, force switch to buff1");
    } else if ((uartCfg->rxBuffState == UART_BUFF_STATE_BUFF1_RECEIVING) &&
        (uartCfg->rxBuff1.pos >= uartCfg->rxBuff1.size) && (uartCfg->rxBuff0.pos > 0)) {
        uartReceiveToIdle(uartCfg, &uartCfg->rxBuff0);
        uartCfg->rxBuffState = UART_BUFF_STATE_BUFF0_RECEIVING;
        PRINT_WARN("current buff1 is full, but other buff0 is not empty, force switch to buff0");
    }

}

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
    PRINT_FOR_TEST("noting to do");        
}

/**
 * 如果传输模式不是HAL_UART_RECEPTION_TOIDLE才走HAL_UART_RxCpltCallback回调，否则走的是HAL_UARTEx_RxEventCallback回调
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    UartConfig_t *comm= getUartConfigByFuncID(UART_FUNC_ID_PC_COMMUNICATION);
    if (huart->Instance == comm->uart->Instance) {
        uartBuffSwitch(comm);    
        vTaskNotifyGiveFromISR(g_cmdTaskHandle, 0); // Notify the command task
    }
}

/**
 * 如果传输模式不是HAL_UART_RECEPTION_TOIDLE才走HAL_UART_RxHalfCpltCallback回调，否则走的是HAL_UARTEx_RxEventCallback回调
 */
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
    PRINT_FOR_TEST("noting to do");
}

/**
 * 注意点一: 如果传输模式是HAL_UART_RECEPTION_TOIDLE，传输过半\传输完成\传输IDLE都会走HAL_UARTEx_RxEventCallback回调
 * 注意点二: 在HAL_UARTEx_RxEventCallback中无法获取到触发中断的状态，因为进入该函数时，中断标志位已经被清除
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    UartConfig_t *comm= getUartConfigByFuncID(UART_FUNC_ID_PC_COMMUNICATION);
    if ((huart->Instance == comm->uart->Instance)) {
        uartBuffSwitch(comm);      
        vTaskNotifyGiveFromISR(g_cmdTaskHandle, 0); // Notify the command task
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    PRINT_ERROR("UART error: %d", huart->ErrorCode);

    UartConfig_t *comm= getUartConfigByFuncID(UART_FUNC_ID_PC_COMMUNICATION);
    if (huart->Instance == comm->uart->Instance) {
        uartBuffSwitch(comm);    
        vTaskNotifyGiveFromISR(g_cmdTaskHandle, 0); // Notify the command task
    }
}
