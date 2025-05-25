/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-25
 * @version v1.0.0
 * @brief UART
 **/

 #ifndef _DMA_CONFIG_H
 #define _DMA_CONFIG_H

#include "nvic_config.h"

#include "stm32f1xx.h"

typedef struct {
    DMA_HandleTypeDef *dma;
    NvicConfig_t nvicConfig;
    uint32_t funcId;
    uint8_t status;
} DmaConfig_t;

enum {
    DMA_FUNC_ID_USART1_TX = 0,
    DMA_FUNC_ID_USART1_RX,
};

DmaConfig_t *getDmaConfigByFuncID(uint32_t funcId);
DMA_HandleTypeDef *getDmaHandleByFuncID(uint32_t funcId);
void dmaInit(void);
 #endif // _DMA_CONFIG_H