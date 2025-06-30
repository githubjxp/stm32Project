/**
 * @file spi_it.c
 * @author jiangxiaoping
 * @date 2025-07-01
 * @version v1.0.0
 */

#include "spi_it.h"

#include "print.h"
#include "spi_config.h"

void SPI2_IRQHandler(void)
{
    HAL_SPI_IRQHandler(SPI2);
}

// (++) HAL_SPI_TxCpltCallback()
// (++) HAL_SPI_RxCpltCallback()
// (++) HAL_SPI_TxRxCpltCallback()
// (++) HAL_SPI_ErrorCallback()
// (++) HAL_SPI_AbortCpltCallback()

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    SPI_Config_t *spiCfg = getSpiConfigByFuncID(SPI_FUNC_ID_FLASH_W25Q128);
    if (spiCfg && spiCfg->funcId == SPI_FUNC_ID_FLASH_W25Q128) {
        PRINT_INFO("SPI2 Rx Complete Callback");
        spiCfg->status = HAL_OK;
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    PRINT_ERROR("SPI2 Error Callback: %d", hspi->ErrorCode);
}