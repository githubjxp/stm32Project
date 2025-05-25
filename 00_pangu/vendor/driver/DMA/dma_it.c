/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-28
 * @version v1.0.0
 * @brief 
 **/

#include "dma_it.h"

#include "dma_config.h"
#include "print.h"
#include "led.h"
#include "stm32f1xx.h"

void DMA1_Channel4_IRQHandler(void)
{
    DMA_HandleTypeDef *hdma = getDmaHandleByFuncID(DMA_FUNC_ID_USART1_TX);
    // PRINT_ERROR("dma tx irq, sr = 0x%x", DMA1->ISR);
    if (hdma != NULL) {
        if (__HAL_DMA_GET_FLAG(hdma, __HAL_DMA_GET_TC_FLAG_INDEX(hdma)) != RESET)
            PRINT_FOR_TEST("dma tx complete flag set");
        else if (__HAL_DMA_GET_FLAG(hdma, __HAL_DMA_GET_HT_FLAG_INDEX(hdma)) != RESET)
            PRINT_FOR_TEST("dma tx half complete flag set");
        HAL_DMA_IRQHandler(hdma);
    }
}

void DMA1_Channel5_IRQHandler(void)
{
    /**
     * callback process, with DMA_IT_TC as an example:
     * 1. clear the DMA_IT_TC flag
     * 2. hdma->XferCpltCallback(hdma)
     * 3. UART_DMAReceiveCplt(hdma)
     * 4. huart->RxCpltCallback(huart)
     * 5. HAL_UART_RxCpltCallback(huart)
     */
    DMA_HandleTypeDef *hdma = getDmaHandleByFuncID(DMA_FUNC_ID_USART1_RX);
    if (hdma != NULL)
        HAL_DMA_IRQHandler(hdma);
}