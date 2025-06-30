/**
 * @file spi_config.h
 * @author jiangxiaoping
 * @date 2025-07-01
 * @version v1.0.0
 */

#ifndef _SPI_CONFIG_H
#define _SPI_CONFIG_H

#include "nvic_config.h"

#include "stm32f1xx.h"

typedef struct {
    SPI_HandleTypeDef *spi;
    NvicConfig_t nvicConfig;
    uint32_t dmaTxFuncId;
    uint32_t dmaRxFuncId;
    uint32_t funcId;
    uint8_t status;
} SPI_Config_t;

typedef enum {
    SPI_FUNC_ID_FLASH_W25Q128 = 0,
    SPI_FUNC_ID_INFRARED_NRF24L01 = SPI_FUNC_ID_FLASH_W25Q128,
} SpiFuncId_t;

SPI_Config_t *getSpiConfigByFuncID(uint32_t funcId);
SPI_HandleTypeDef *getSpiHandleByFuncID(uint32_t funcId);
void spiInit(void);

#endif /* _SPI_CONFIG_H */