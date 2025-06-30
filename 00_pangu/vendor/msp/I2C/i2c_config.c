/**
 * @file i2c_config.c
 * @author jiangxiaoping
 * @date 2025-06-12
 * @version v1.0.0
 **/

#include "i2c_config.h"

#include "DMA_config.h"
#include "NVIC_config.h"

static I2C_HandleTypeDef i2c[] = {
     {
        .Instance = I2C1,
        .Init.ClockSpeed = 400000, // 400kHz
        .Init.DutyCycle = I2C_DUTYCYCLE_16_9,
        .Init.OwnAddress1 = 0xA0, // 7位OAR1[1:7]为地址，10位OAR1[0:9]为地址
        .Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT,
        .Init.DualAddressMode = I2C_DUALADDRESS_DISABLE, // 相当于检查OAR2[0]为1还是为0
        .Init.OwnAddress2 = 0xB0, // 只支持配成7位地址，不支持配为10位地址，OAR2[1:7]为地址，OAR2[0]=1, OAR2可以被识别
        .Init.GeneralCallMode = I2C_GENERALCALL_DISABLE, // 是否允许通用调用地址，通常是0x00
        .Init.NoStretchMode = I2C_NOSTRETCH_DISABLE, // 是否允许时钟拉伸, disable=允许
     },
};

static I2cConfig_t g_i2cConfigTbl[] = {
    {
        .i2c = &i2c[0],
        .nvicConfig = {
            [I2C_IT_IDX_EV] = {
                .irqn = I2C1_EV_IRQn,
                .preemptPriority = NVIC_PRIORITY_I2C1_EV,
                .subPriority = 0,
            },
            [I2C_IT_IDX_ER] = {
                .irqn = I2C1_ER_IRQn,
                .preemptPriority = NVIC_PRIORITY_I2C1_ER,
                .subPriority = 0,
            },
        },
        .dmaTxFuncId = DMA_FUNC_ID_I2C1_TX,
        .dmaRxFuncId = DMA_FUNC_ID_I2C1_RX,
        .i2cFuncId = I2C_FUNC_ID_MPU6050,
        .status = HAL_ERROR,
    },
};

I2cConfig_t *getI2cConfigByFuncID(uint32_t funcId) {
    for (int i = 0; i < sizeof(g_i2cConfigTbl) / sizeof(g_i2cConfigTbl[0]); i++) {
        if (g_i2cConfigTbl[i].i2cFuncId == funcId) {
            return &g_i2cConfigTbl[i];
        }
    }
    return NULL;
}

I2C_HandleTypeDef *getI2cHandleByFuncID(uint32_t funcId) {
    for (int i = 0; i < sizeof(g_i2cConfigTbl) / sizeof(g_i2cConfigTbl[0]); i++) {
        if (g_i2cConfigTbl[i].i2cFuncId == funcId) {
            return g_i2cConfigTbl[i].i2c;
        }
    }
    return NULL;
}

static void enableI2cClk(I2C_TypeDef *I2Cx) {
    if (I2Cx == I2C1) {
        __HAL_RCC_I2C1_CLK_ENABLE();
    } else if (I2Cx == I2C2) {
        __HAL_RCC_I2C2_CLK_ENABLE();
    } else {
        // Add more I2C instances if needed
    }
}

void i2cInit(void) {
    int i;
    int ret;

    for (i = 0; i < sizeof(g_i2cConfigTbl) / sizeof(g_i2cConfigTbl[0]); i++) {
        // enable I2C clock
        enableI2cClk(g_i2cConfigTbl[i].i2c->Instance);

        // Initialize the I2C peripheral
        ret = HAL_I2C_Init(g_i2cConfigTbl[i].i2c);
        if (ret != HAL_OK) {
            g_i2cConfigTbl[i].status = HAL_ERROR;
            continue;
        } else {
            // NVIC init
            HAL_NVIC_SetPriority(g_i2cConfigTbl[i].nvicConfig[I2C_IT_IDX_EV].irqn,
                                 g_i2cConfigTbl[i].nvicConfig[I2C_IT_IDX_EV].preemptPriority,
                                 g_i2cConfigTbl[i].nvicConfig[I2C_IT_IDX_EV].subPriority);
            HAL_NVIC_EnableIRQ(g_i2cConfigTbl[i].nvicConfig[I2C_IT_IDX_EV].irqn);

            HAL_NVIC_SetPriority(g_i2cConfigTbl[i].nvicConfig[I2C_IT_IDX_ER].irqn,
                                 g_i2cConfigTbl[i].nvicConfig[I2C_IT_IDX_ER].preemptPriority,
                                 g_i2cConfigTbl[i].nvicConfig[I2C_IT_IDX_ER].subPriority);
            HAL_NVIC_EnableIRQ(g_i2cConfigTbl[i].nvicConfig[I2C_IT_IDX_ER].irqn);

            // DMA init
            DMA_HandleTypeDef *dmaTx = getDmaHandleByFuncID(g_i2cConfigTbl[i].dmaTxFuncId);
            DMA_HandleTypeDef *dmaRx = getDmaHandleByFuncID(g_i2cConfigTbl[i].dmaRxFuncId);
            if (dmaTx != NULL) {
                g_i2cConfigTbl[i].i2c->hdmatx = dmaTx; // 待优化：移除该逻辑，预期不影响代码逻辑
                __HAL_LINKDMA(g_i2cConfigTbl[i].i2c, hdmatx, *dmaTx);
            }
            if (dmaRx != NULL) {
                g_i2cConfigTbl[i].i2c->hdmarx = dmaRx; // 待优化：移除该逻辑，预期不影响代码逻辑
                __HAL_LINKDMA(g_i2cConfigTbl[i].i2c, hdmarx, *dmaRx);
            }

            __HAL_I2C_ENABLE(g_i2cConfigTbl[i].i2c); // 待优化：移除该逻辑，预期不影响代码逻辑

            g_i2cConfigTbl[i].status = HAL_OK;
        }
    }
}
