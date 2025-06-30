/**
 * @file spi_config.c
 * @author jiangxiaoping
 * @date 2025-07-01
 * @version v1.0.0
 */

#include "spi_config.h"

#include "print.h"
#include "dma_config.h"

#include "stm32f1xx.h"

static SPI_HandleTypeDef spi[] = {
    {
        .Instance = SPI2,
        .Init.Mode = SPI_MODE_MASTER,
        .Init.Direction = SPI_DIRECTION_2LINES,
        .Init.DataSize = SPI_DATASIZE_8BIT,
        .Init.CLKPolarity = SPI_POLARITY_HIGH, // SPI[2] = CPOL HIGH, CPHA 1EDGE
        .Init.CLKPhase = SPI_PHASE_1EDGE, // SPI[2] = CPOL HIGH, CPHA 1EDGE
        .Init.NSS = SPI_NSS_SOFT,
        .Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32, // 1.125MHz for 36MHz APB1 clock
        .Init.FirstBit = SPI_FIRSTBIT_MSB,
        .Init.TIMode = SPI_TIMODE_DISABLE,
        .Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE,
        .Init.CRCPolynomial = 10,
    },
};

static SPI_Config_t g_spiConfigTbl[] = {
    {
        .spi = &spi[0],
        .nvicConfig = {
            .irqn = SPI2_IRQn,
            .preemptPriority = NVIC_PRIORITY_SPI2,
            .subPriority = 0,
        },
        .dmaTxFuncId = DMA_FUNC_ID_SPI2_TX,
        .dmaRxFuncId = DMA_FUNC_ID_SPI2_RX,
        .funcId = SPI_FUNC_ID_FLASH_W25Q128,
        .status = HAL_ERROR,
    },
};

SPI_Config_t *getSpiConfigByFuncID(uint32_t funcId) {
    for (int i = 0; i < sizeof(g_spiConfigTbl) / sizeof(g_spiConfigTbl[0]); i++) {
        if (g_spiConfigTbl[i].funcId == funcId) {
            return &g_spiConfigTbl[i];
        }
    }
    return NULL;
}

SPI_HandleTypeDef *getSpiHandleByFuncID(uint32_t funcId) {
    for (int i = 0; i < sizeof(g_spiConfigTbl) / sizeof(g_spiConfigTbl[0]); i++) {
        if (g_spiConfigTbl[i].funcId == funcId) {
            return g_spiConfigTbl[i].spi;
        }
    }
    return NULL;
}

static void enableSpiClk(SPI_TypeDef *SPIx) {
    if (SPIx == SPI1) {
        __HAL_RCC_SPI1_CLK_ENABLE();
    } else if (SPIx == SPI2) {
        __HAL_RCC_SPI2_CLK_ENABLE();
    } else if (SPIx == SPI3) {
        __HAL_RCC_SPI3_CLK_ENABLE();
    } else {
        PRINT_ERROR("Unsupported SPI instance: %p", SPIx);
    }
}

void spiInit(void) {
    int i;
    int ret;

    for (i = 0; i < sizeof(g_spiConfigTbl) / sizeof(g_spiConfigTbl[0]); i++) {
        // Enable SPI clock
        enableSpiClk(g_spiConfigTbl[i].spi->Instance);

        // Initialize the SPI peripheral
        ret = HAL_SPI_Init(g_spiConfigTbl[i].spi);
        if (ret != HAL_OK) {
            PRINT_ERROR("Failed to initialize SPI: %d", ret);
            g_spiConfigTbl[i].status = HAL_ERROR;
            continue;
        }

        // NVIC Init
        HAL_NVIC_SetPriority(g_spiConfigTbl[i].nvicConfig.irqn, g_spiConfigTbl[i].nvicConfig.preemptPriority, 
                            g_spiConfigTbl[i].nvicConfig.subPriority);
        HAL_NVIC_EnableIRQ(g_spiConfigTbl[i].nvicConfig.irqn);

        // DMA Init
        DmaConfig_t *dmaTxConfig = getDmaConfigByFuncID(g_spiConfigTbl[i].dmaTxFuncId);
        DmaConfig_t *dmaRxConfig = getDmaConfigByFuncID(g_spiConfigTbl[i].dmaRxFuncId);
        if (dmaTxConfig != NULL)
            __HAL_LINKDMA(g_spiConfigTbl[i].spi, hdmatx, *dmaTxConfig->dma);
        if (dmaRxConfig != NULL)
            __HAL_LINKDMA(g_spiConfigTbl[i].spi, hdmarx, *dmaRxConfig->dma);

        // Set status to OK
        g_spiConfigTbl[i].status = HAL_OK;
    }
}