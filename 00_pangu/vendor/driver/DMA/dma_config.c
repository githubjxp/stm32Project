/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-25
 * @version v1.0.0
 * @brief UART
 **/

#include "dma_config.h"

#include "stm32f1xx.h"

#include "print.h"

static DMA_HandleTypeDef hdma[] = {
    {
        .Instance = DMA1_Channel4,
        .Init.Direction = DMA_MEMORY_TO_PERIPH,
        .Init.PeriphInc = DMA_PINC_DISABLE,
        .Init.MemInc = DMA_MINC_ENABLE,
        .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
        .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
        .Init.Mode = DMA_NORMAL,
        .Init.Priority = DMA_PRIORITY_MEDIUM,
    },
    {
        .Instance = DMA1_Channel5,
        .Init.Direction = DMA_PERIPH_TO_MEMORY,
        .Init.PeriphInc = DMA_PINC_DISABLE,
        .Init.MemInc = DMA_MINC_ENABLE,
        .Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
        .Init.MemDataAlignment = DMA_MDATAALIGN_BYTE,
        .Init.Mode = DMA_NORMAL,
        .Init.Priority = DMA_PRIORITY_MEDIUM,
    },
};

static DmaConfig_t g_dmaConfigTbl[] = {
    {
        .dma = &hdma[0],
        .nvicConfig = {
            .irqn = DMA1_Channel4_IRQn,
            .preemptPriority = NVIC_PRIORITY_DMA1_CHANNEL4,
            .subPriority = 0,
        },
        .funcId = DMA_FUNC_ID_USART1_TX,
        .status = HAL_NO_INITIALIZATION,
    },
    {
        .dma = &hdma[1],
        .nvicConfig = {
            .irqn = DMA1_Channel5_IRQn,
            .preemptPriority = NVIC_PRIORITY_DMA1_CHANNEL5,
            .subPriority = 0,
        },
        .funcId = DMA_FUNC_ID_USART1_RX,
        .status = HAL_NO_INITIALIZATION,
    },
};

DmaConfig_t *getDmaConfigByFuncID(uint32_t funcId)
{
    for (int i = 0; i < sizeof(g_dmaConfigTbl) / sizeof(g_dmaConfigTbl[0]); i++) {
        if (g_dmaConfigTbl[i].funcId == funcId)
            return &g_dmaConfigTbl[i];
    }

    return NULL;
}

DMA_HandleTypeDef *getDmaHandleByFuncID(uint32_t funcId)
{
    for (int i = 0; i < sizeof(g_dmaConfigTbl) / sizeof(g_dmaConfigTbl[0]); i++) {
        if (g_dmaConfigTbl[i].funcId == funcId)
            return g_dmaConfigTbl[i].dma;
    }

    return NULL;
}

static void dmaClkEnable(DMA_HandleTypeDef *hdma)
{
    if ((hdma->Instance == DMA1_Channel1) || (hdma->Instance == DMA1_Channel2) ||
        (hdma->Instance == DMA1_Channel3) || (hdma->Instance == DMA1_Channel4) ||
        (hdma->Instance == DMA1_Channel5) || (hdma->Instance == DMA1_Channel6) ||
        (hdma->Instance == DMA1_Channel7)) {
        __HAL_RCC_DMA1_CLK_ENABLE();
    } else if ((hdma->Instance == DMA2_Channel1) || (hdma->Instance == DMA2_Channel2) ||
               (hdma->Instance == DMA2_Channel3) || (hdma->Instance == DMA2_Channel4) ||
               (hdma->Instance == DMA2_Channel5)) {
        __HAL_RCC_DMA2_CLK_ENABLE();
    }
}

void dmaInit(void)
{
    int i;
    int ret;
    
    for (i = 0; i < sizeof(g_dmaConfigTbl) / sizeof(g_dmaConfigTbl[0]); i++) {
        dmaClkEnable(g_dmaConfigTbl[i].dma);
        ret = HAL_DMA_Init(g_dmaConfigTbl[i].dma);
        if (ret != HAL_OK) {
            g_dmaConfigTbl[i].status = HAL_ERROR;
            continue;
        } else {
            g_dmaConfigTbl[i].status = HAL_OK;
        }

        HAL_NVIC_SetPriority(g_dmaConfigTbl[i].nvicConfig.irqn,
            g_dmaConfigTbl[i].nvicConfig.preemptPriority, g_dmaConfigTbl[i].nvicConfig.subPriority);
        HAL_NVIC_EnableIRQ(g_dmaConfigTbl[i].nvicConfig.irqn);
    }
}