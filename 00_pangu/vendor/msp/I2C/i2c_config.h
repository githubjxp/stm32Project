/**
 * @file i2c_config.h
 * @author jiangxiaoping
 * @date 2025-06-12
 * @version v1.0.0
 */

#ifndef _I2C_CONFIG_H
#define _I2C_CONFIG_H

#include "nvic_config.h"

#include "stm32f1xx.h"

#define I2C_IT_IDX_EV 0
#define I2C_IT_IDX_ER 1
#define I2C_IT_NUM 2

typedef struct {
    I2C_HandleTypeDef *i2c;
    NvicConfig_t nvicConfig[I2C_IT_NUM]; // 0: I2C1_EV_IRQn, 1: I2C1_ER_IRQn
    uint32_t dmaTxFuncId;
    uint32_t dmaRxFuncId;
    uint32_t i2cFuncId;
    uint8_t status;
} I2cConfig_t;

typedef enum {
    I2C_FUNC_ID_MPU6050 = 0,
} I2cFuncId_t;

#define I2C1_ADDR_YWDZ (0x40 << 1) // 优为电子I2C从机地址： 0x40, 左移1位是因为I2C地址是7位的，最后一位是读写位

I2cConfig_t *getI2cConfigByFuncID(uint32_t funcId);
I2C_HandleTypeDef *getI2cHandleByFuncID(uint32_t funcId);
void i2cInit(void);
#endif