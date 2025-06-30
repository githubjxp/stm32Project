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

/*
 * DMA_FUNC_ID_USART1_TX
 */
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

/*
 * DMA_FUNC_ID_USART1_RX
 */
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
    if (hdma != NULL) {
        
         // GET_IT_SOURCE宏是用来获取中断源使能的，而不是当前触发的中断
        // if (__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_HT) != RESET) {

        // 接收传输过半中断不处理，避免uart dma接收时切换buff
        if (__HAL_DMA_GET_FLAG(hdma, __HAL_DMA_GET_HT_FLAG_INDEX(hdma)) != RESET) {
            PRINT_FOR_TEST("dma rx half complete flag set");
            __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_HT_FLAG_INDEX(hdma)); // DMA向IFCR寄存器中写1，清除ISR中的中断标志
            return;
        }
        HAL_DMA_IRQHandler(hdma);
    }
}