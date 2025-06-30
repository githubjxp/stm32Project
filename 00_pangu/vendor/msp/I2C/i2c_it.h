/**
 * @file i2c_it.h
 * @author jiangxiaoping
 * @date 2025-06-12
 * @version v1.0.0
 */

#ifndef _I2C_IT_H
#define _I2C_IT_H

#include "stm32f1xx_hal.h"

void I2C1_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);

#endif /* _I2C_IT_H */