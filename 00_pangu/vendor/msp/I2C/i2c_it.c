/**
 * @file i2c_it.c
 * @author jiangxiaoping
 * @date 2025-06-12
 * @version v1.0.0
 * @brief I2C interrupt handlers
 *        PEC: Packet Error Checking, used for error detection in I2C communication.
 *        ITERREN: TIMEOUT, ARLO(仲裁丢失), BERR(总线出错), OVR(过载), PECERR(接收时发生PEC错误), AF(应答失败)
 *        ITEVTEN: ADDR(地址已被发送), STOPF(检测到停止条件), BTF(字节发送结束), SB(起始位), ADD10(10位地址已被发送),
 *        ITBUFEN: RXNE(接收缓冲区非空), TXE(发送缓冲区空)
 */

#include "i2c_it.h"

#include "i2c_config.h"
#include "print.h"

#include "stm32f1xx.h"

static void ClearAllI2CItFlags(I2C_HandleTypeDef *hi2c)
{
    // 清除 AF (Acknowledge Failure) flag
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);

    // 清除 STOPF (Stop Detection) flag
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);

    // 清除 ADD10 (10-bit Header Sent) flag
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_ADD10);

    // 清除 BTF (Byte Transfer Finished) flag
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_BTF);

    // 清除 ADDR (Address Sent/Matched) flag
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_ADDR);

    // 清除 TXE (Data Register Empty) flag
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_TXE);

    // 清除 RXNE (Data Register Not Empty) flag
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_RXNE);
}


void I2C1_EV_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(I2C1);
}

void I2C1_ER_IRQHandler(void)
{
    HAL_I2C_ER_IRQHandler(I2C1);
}

// (++) HAL_I2C_MasterTxCpltCallback()
// (++) HAL_I2C_MasterRxCpltCallback()
// (++) HAL_I2C_SlaveTxCpltCallback()
// (++) HAL_I2C_SlaveRxCpltCallback()
// (++) HAL_I2C_MemTxCpltCallback()
// (++) HAL_I2C_MemRxCpltCallback()
// (++) HAL_I2C_AddrCallback()
// (++) HAL_I2C_ListenCpltCallback()
// (++) HAL_I2C_ErrorCallback()
// (++) HAL_I2C_AbortCpltCallback()

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    PRINT_ERROR("I2C Error Callback: %d", hi2c->ErrorCode);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    I2cConfig_t *i2cCfg = getI2cConfigByFuncID(I2C_FUNC_ID_MPU6050);
    if (i2cCfg == NULL || i2cCfg->i2c == NULL) {
        return;
    }

    if (hi2c->Instance == i2cCfg->i2c->Instance) {
        PRINT_INFO("I2C Master Rx Complete");
    }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    I2cConfig_t *i2cCfg = getI2cConfigByFuncID(I2C_FUNC_ID_MPU6050);
    if (i2cCfg == NULL || i2cCfg->i2c == NULL) {
        return;
    }

    if (hi2c->Instance == i2cCfg->i2c->Instance) {
        PRINT_INFO("I2C Memory Rx Complete");
    }
}
