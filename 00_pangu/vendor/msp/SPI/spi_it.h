/**
 * @file spi_it.h
 * @author jiangxiaoping
 * @date 2025-07-01
 * @version v1.0.0
 */

#ifndef SPI_IT_H
#define SPI_IT_H

#include "stm32f1xx.h"

void SPI1_IRQHandler(void);
void SPI2_IRQHandler(void);
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);

#endif /* SPI_IT_H */