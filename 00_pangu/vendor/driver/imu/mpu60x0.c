/**
 * @file mpu60x0.C
 * @author jiangxiaoping
 * @date 2025-07-07
 * @version v1.0.0
 */

#include "mpu60x0.h"

#include "bus.h"
#include "hw_interface.h"
#include "imu.h"
#include "print.h"

#define MPU60X0_AXIS_COUNT 3
#define MPU60X0_X_AXIS 0
#define MPU60X0_Y_AXIS 1
#define MPU60X0_Z_AXIS 2
#define MPU60X0_ACC_DATA_LEN         6
#define MPU60X0_GYRO_DATA_LEN        6
#define MPU60X0_TEMPERATURE_DATA_LEN 2

typedef enum {
    X_AXIS_H_IDX = 0,
    X_AXIS_L_IDX,
    Y_AXIS_H_IDX,
    Y_AXIS_L_IDX,
    Z_AXIS_H_IDX,
    Z_AXIS_L_IDX,
} axis_idx_t;

typedef enum {
    TEMPERATURE_H_IDX = 0,
    TEMPERATURE_L_IDX,
} temperature_idx_t;

void mpu60x0SelfTest(Driver_t *driver) {
    if (driver == NULL) {
        PRINT_ERROR("Invalid driver!");
        return;
    }

    uint8_t regData;
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_ACCEL_CONFIG, &regData);

    regData |= (MPU60X0_XACCEL_SELF_TEST | MPU60X0_YACCEL_SELF_TEST | MPU60X0_ZACCEL_SELF_TEST);

    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_ACCEL_CONFIG, regData);
    
    PRINT_INFO("MPU60x0 self test completed");
}

/*
 * @brief MPU60x0复位设备
 * @param driver driver总线句柄
 * @return 成功返回SUCCESS, 失败返回ERROR
 */
int mpu60x0Reset(Driver_t *driver) {
    if (driver == NULL) {
        PRINT_ERROR("Invalid driver!");
        return ERROR;
    }

    uint8_t regValue;
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_PWR_MGMT_1, &regValue);

    regValue |= MPU60X0_DEVICE_RESET; // 设置设备复位位
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_PWR_MGMT_1, regValue);
    PRINT_INFO("MPU60x0 reset!");
    return SUCCESS;
}

/*
 * @brief MPU60x0设置待机模式
 * @param driver driver总线句柄
 * @param stby_axis 待机轴，1：待机，0：工作
 * @return 成功返回SUCCESS, 失败返回ERROR
 */
int mpu60x0SetStandbyMode(Driver_t *driver, uint8_t stby_axis) {
    if (driver == NULL) {
        PRINT_ERROR("Invalid driver!");
        return ERROR;
    }

    uint8_t regValue;
    regValue = (MPU60X0_STBY_AXIS_MASK & stby_axis) | MPU60X0_LP_WAKE_CTRL_40HZ;

    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_PWR_MGMT_1, regValue);
    PRINT_INFO("MPU60x0 %s standby mode!, axis[0x%x]", stby_axis ? "enter" : "exit", stby_axis);
    return SUCCESS;
}

/*
 * @brief MPU60x0设置睡眠模式
 * @param driver driver总线句柄
 * @param sleep true: 进入睡眠模式, false: 退出睡眠模式
 * @return 成功返回SUCCESS, 失败返回ERROR
 */
int mpu60x0SetSleepMode(Driver_t *driver, bool sleep) {
    if (driver == NULL) {
        PRINT_ERROR("Invalid driver!");
        return ERROR;
    }

    uint8_t regValue;
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_PWR_MGMT_1, &regValue);
    if (sleep)
        regValue |= MPU60X0_SLEEP;
    else
        regValue &= ~MPU60X0_SLEEP;

    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_PWR_MGMT_1, regValue);
    PRINT_INFO("MPU60x0 %s sleep mode!", sleep ? "enter" : "exit");
    return SUCCESS;
}

/*
 * @brief 读取MPU60x0 acc的分辨率
 * @param driver driver总线句柄
 * @param resolution 分辨率的指针
 * @return 成功返回SUCCESS, 失败返回ERROR
 * @note 分辨率单位为ng/LSB
 */
int mpu60x0GetAccResolution(Driver_t *driver, float *resolution) {
    if (driver == NULL || resolution == NULL) {
        PRINT_ERROR("Invalid parameters!");
        return ERROR;
    }

    uint8_t accConfig;
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_ACCEL_CONFIG, &accConfig);

    // 获取加速度计的分辨率
    switch (accConfig & MPU60X0_ACCEL_CONFIG_FS_MASK) {
        case MPU60X0_ACCEL_CONFIG_FS_2G:
            *resolution = 61.036f; // 61.036 = 2000000ng * 2 / 65536
            break;
        case MPU60X0_ACCEL_CONFIG_FS_4G:
            *resolution = 122.072f; // 122.072 = 4000000ng * 2 / 65536
            break;
        case MPU60X0_ACCEL_CONFIG_FS_8G:
            *resolution = 244.144f; // 244.144 = 8000000ng * 2 / 65536
            break;
        case MPU60X0_ACCEL_CONFIG_FS_16G:
            *resolution = 488.281f; // 488.281 = 16000000ng * 2 / 65536
            break;
        default:
            PRINT_ERROR("Unsupported accelerometer range!");
            return ERROR;
    }

    return SUCCESS;
}

/*
 * @brief 读取MPU60x0 gyro的分辨率
 * @param driver driver总线句柄
 * @param resolution 分辨率的指针
 * @return 成功返回SUCCESS, 失败返回ERROR
 * @note 分辨率单位为mdps/LSB
 */
int mpu60x0GetGyroResolution(Driver_t *driver, float *resolution) {
    if (driver == NULL || resolution == NULL) {
        PRINT_ERROR("Invalid parameters!");
        return ERROR;
    }

    uint8_t gyroConfig;
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_GYRO_CONFIG, &gyroConfig);

    // 获取陀螺仪的分辨率
    switch (gyroConfig & MPU60X0_GYRO_CONFIG_FS_MASK) {
        case MPU60X0_GYRO_CONFIG_FS_250DPS:
            *resolution = 7.6294f; // 7.6294 = 250000mdps * 2 / 65536
            break;
        case MPU60X0_GYRO_CONFIG_FS_500DPS:
            *resolution = 15.2588f; // 15.2588 = 500000mdps * 2 / 65536
            break;
        case MPU60X0_GYRO_CONFIG_FS_1000DPS:
            *resolution = 30.5178f; // 30.5178 = 1000000mdps * 2 / 65536
            break;
        case MPU60X0_GYRO_CONFIG_FS_2000DPS:
            *resolution = 61.0352f; // 61.0352 = 2000000mdps * 2 / 65536
            break;
        default:
            PRINT_ERROR("Unsupported gyroscope range!");
            return ERROR;
    }

    return SUCCESS;
}

/*
 * @brief 读取MPU60x0 acc的灵敏度
 * @param driver driver总线句柄
 * @param sensitivity 灵敏度的指针
 * @return 成功返回SUCCESS, 失败返回ERROR
 * @note 灵敏度单位为LSB/g
 */
int mpu60x0GetAccSensitivity(Driver_t *driver, int *sensitivity) {
    if (driver == NULL || sensitivity == NULL) {
        PRINT_ERROR("Invalid parameters!");
        return ERROR;
    }

    uint8_t accConfig;
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_ACCEL_CONFIG, &accConfig);

    // 获取加速度计的灵敏度
    switch (accConfig & MPU60X0_ACCEL_CONFIG_FS_MASK) {
        case MPU60X0_ACCEL_CONFIG_FS_2G:
            *sensitivity = 16384; // 16384 LSB/g
            break;
        case MPU60X0_ACCEL_CONFIG_FS_4G:
            *sensitivity = 8192; // 8192 LSB/g
            break;
        case MPU60X0_ACCEL_CONFIG_FS_8G:
            *sensitivity = 4096; // 4096 LSB/g
            break;
        case MPU60X0_ACCEL_CONFIG_FS_16G:
            *sensitivity = 2048; // 2048 LSB/g
            break;
        default:
            PRINT_ERROR("Unsupported accelerometer range!");
            return ERROR;
    }

    return SUCCESS;
}

/*
 * @brief 读取MPU60x0 gyro的灵敏度
 * @param driver driver总线句柄
 * @param sensitivity 灵敏度的指针
 * @return 成功返回SUCCESS, 失败返回ERROR
 * @note 灵敏度单位为LSB/(°/s)
 */
int mpu60x0GetGyroSensitivity(Driver_t *driver, float *sensitivity) {
    if (driver == NULL || sensitivity == NULL) {
        PRINT_ERROR("Invalid parameters!");
        return ERROR;
    }

    uint8_t gyroConfig;
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_GYRO_CONFIG, &gyroConfig);

    // 获取陀螺仪的灵敏度
    switch (gyroConfig & MPU60X0_GYRO_CONFIG_FS_MASK) {
        case MPU60X0_GYRO_CONFIG_FS_250DPS:
            *sensitivity = 131.072f; // 131.072 LSB/(°/s)
            break;
        case MPU60X0_GYRO_CONFIG_FS_500DPS:
            *sensitivity = 65.536f; // 65.536 LSB/(°/s)
            break;
        case MPU60X0_GYRO_CONFIG_FS_1000DPS:
            *sensitivity = 32.768f; // 32.768 LSB/(°/s)
            break;
        case MPU60X0_GYRO_CONFIG_FS_2000DPS:
            *sensitivity = 16.384f; // 16.384 LSB/(°/s)
            break;
        default:
            PRINT_ERROR("Unsupported gyroscope range!");
            return ERROR;
    }

    return SUCCESS;
}

/*
 * @brief 读取MPU60x0 acc数据
 * @param driver driver总线句柄
 * @param accX accX轴数据
 * @param accY accY轴数据
 * @param accZ accZ轴数据
 * @return 成功返回SUCCESS, 失败返回ERROR
 * @note accX, accY, accZ的值为16位有符号整数，单位为LSB
 */
int mpu60x0ReadAcc(Driver_t *driver, int16_t *acc_data, uint8_t len) {
    if (driver == NULL || acc_data == NULL || len < MPU60X0_AXIS_COUNT) {
        PRINT_ERROR("Invalid parameters!");
        return ERROR;
    }

    uint8_t data[MPU60X0_ACC_DATA_LEN] = {0};
    int ret;

    // 读取加速度计数据寄存器
    ret = mpu60x0I2cBlockReadReg8(driver->bus, MPU60X0_REG_ACCEL_XOUT_H, data, MPU60X0_ACC_DATA_LEN);
    PRINT_DEBUG("%d, %d, %d, %d, %d, %d", data[0], data[1], data[2], data[3], data[4], data[5]);
    if (ret != SUCCESS) {
        PRINT_ERROR("Failed to read accelerometer data!");
        return ERROR;
    }

    // 将读取的数据转换为16位整数
    acc_data[0] = (int16_t)((data[X_AXIS_H_IDX] << 8) | data[X_AXIS_L_IDX]);
    acc_data[1] = (int16_t)((data[Y_AXIS_H_IDX] << 8) | data[Y_AXIS_L_IDX]);
    acc_data[2] = (int16_t)((data[Z_AXIS_H_IDX] << 8) | data[Z_AXIS_L_IDX]);

    return SUCCESS;
}

/*
 * @brief 读取MPU60x0 gyro数据
 * @param driver driver总线句柄
 * @param gyroX gyro X轴数据
 * @param gyroY gyro Y轴数据
 * @param gyroZ gyro Z轴数据
 * @return 成功返回SUCCESS, 失败返回ERROR
 * @note gyroX, gyroY, gyroZ的值为16位有符号整数，单位为LSB
 */
int mpu60x0ReadGyro(Driver_t *driver, int16_t *gyro_data, uint8_t len) {
    if (driver == NULL || gyro_data == NULL || len < MPU60X0_AXIS_COUNT) {
        PRINT_ERROR("Invalid parameters!");
        return ERROR;
    }

    uint8_t data[MPU60X0_GYRO_DATA_LEN] = {0};
    int ret;

    // 读取陀螺仪数据寄存器
    ret = mpu60x0I2cBlockReadReg8(driver->bus, MPU60X0_REG_GYRO_XOUT_H, data, MPU60X0_GYRO_DATA_LEN);
    if (ret != SUCCESS) {
        PRINT_ERROR("Failed to read gyroscope data!");
        return ERROR;
    }

    // 将读取的数据转换为16位整数
    gyro_data[0] = (int16_t)((data[X_AXIS_H_IDX] << 8) | data[X_AXIS_L_IDX]);
    gyro_data[1] = (int16_t)((data[Y_AXIS_H_IDX] << 8) | data[Y_AXIS_L_IDX]);
    gyro_data[2] = (int16_t)((data[Z_AXIS_H_IDX] << 8) | data[Z_AXIS_L_IDX]);

    return SUCCESS;
}

#define MPU60X0_TEMPERATURE_LSB 340 // 温度传感器每1°C对应的LSB值
#define MPU60X0_TEMPERATURE_OFFSET -521 // 温度传感器偏移LSB值，35°C对应-521 LSB
#define MPU60X0_REG_ZERO_TEMPERATURE 36.53f // 温度传感器寄存器0值对应的温度，单位为°C，即 35 + 521 / 340
#define C_TO_1P100C_FACTOR 100 // 摄氏度转换为1/100°C的系数
/*
 * @brief 读取MPU60x0 temperature数据
 * @param driver driver总线句柄
 * @param temperature temperature数据
 * @return 成功返回SUCCESS, 失败返回ERROR
 * @note temperature的值为16位有符号整数，单位为1/100°C, 340 LSB = 1°C, 35°C = -521 LSB， 521 / 340 = 1.53°C
 * @note 温度计算公式为: temperature = ((raw_temp / 340.0) + 36.53) * 100; // 36.53 = 35 + 521 / 340
 */
int mpu60x0ReadTemperature(Driver_t *driver, int16_t *temperature) {
    uint8_t data[MPU60X0_TEMPERATURE_DATA_LEN] = {0};
    int ret;

    // 读取温度数据寄存器
    ret = mpu60x0I2cBlockReadReg8(driver->bus, MPU60X0_REG_TEMP_OUT_H, data, MPU60X0_TEMPERATURE_DATA_LEN);
    if (ret != SUCCESS) {
        PRINT_ERROR("Failed to read temperature data!");
        return ERROR;
    }

    // 将读取的数据转换为摄氏度
    *temperature = ((float)((data[TEMPERATURE_H_IDX] << 8) | data[TEMPERATURE_L_IDX]) / MPU60X0_TEMPERATURE_LSB +
        MPU60X0_REG_ZERO_TEMPERATURE) * C_TO_1P100C_FACTOR;

    return SUCCESS;
}

int32_t mpu60x0CheckDeviceStatus(Driver_t *driver) {
    if (driver == NULL) {
        PRINT_ERROR("Invalid driver!");
        return ERROR;
    }

    uint8_t regValue;

    // 读取WHO_AM_I寄存器
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_WHO_AM_I, &regValue);
    PRINT_INFO("MPU60x0 WHO_AM_I: 0x%x", regValue);

    // MPU60X0_REG_PWR_MGMT_1
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_PWR_MGMT_1, &regValue);
    PRINT_INFO("MPU60x0 PWR_MGMT_1: 0x%x", regValue);

    // MPU60X0_REG_PWR_MGMT_2
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_PWR_MGMT_2, &regValue);
    PRINT_INFO("MPU60x0 PWR_MGMT_2: 0x%x", regValue);

    // MPU60X0_REG_USER_CTRL
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_USER_CTRL, &regValue);
    PRINT_INFO("MPU60x0 USER_CTRL: 0x%x", regValue);

    // MPU60X0_REG_I2C_MST_CTRL
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_I2C_MST_CTRL, &regValue);
    PRINT_INFO("MPU60x0 I2C_MST_CTRL: 0x%x", regValue);

    // MPU60X0_REG_CONFIG
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_CONFIG, &regValue);
    PRINT_INFO("MPU60x0 CONFIG: 0x%x", regValue);

    // MPU60X0_REG_SMPLRT_DIV
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_SMPLRT_DIV, &regValue);
    PRINT_INFO("MPU60x0 SMPLRT_DIV: 0x%x", regValue);

    // MPU60X0_REG_GYRO_CONFIG
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_GYRO_CONFIG, &regValue);
    PRINT_INFO("MPU60x0 GYRO_CONFIG: 0x%x", regValue);

    // MPU60X0_REG_ACCEL_CONFIG
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_ACCEL_CONFIG, &regValue);
    PRINT_INFO("MPU60x0 ACCEL_CONFIG: 0x%x", regValue);

    return SUCCESS;
}

int32_t mpu60x0Init(Driver_t *driver) {
    uint8_t data[10] = {0};
    int ret;

    // 重置MPU6050寄存器
    // ret = mpu60x0Reset(driver);
    // if (ret != SUCCESS)
    //     return ERROR;
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_PWR_MGMT_1, 0); // 首次需写0? 否则没有ag数据
    delayMs(50); // 等待50ms,确保复位完成

    // 读取WHO_AM_I寄存器,确认设备是否存在
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_WHO_AM_I, &data[0]);
    if (data[0] != MPU60X0_WHO_AM_I_DEFAULT) {
        PRINT_ERROR("MPU6050 not found or WHO_AM_I mismatch!");
        return ERROR;
    }

    // 初始化AUX_VDDIO寄存器, 设置为3.3V
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_AUX_VDDIO, AUX_VDDIO_VLOGIC);

    // 配置PWR_MGMT_1, 使用PLL X轴陀螺仪作为时钟源, 1KHz
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_PWR_MGMT_1, MPU60X0_CLKSEL_PLL_XGYRO);

    // 配置PWR_MGMT_2,  设置低功耗唤醒频率为40Hz
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_PWR_MGMT_2, MPU60X0_LP_WAKE_CTRL_40HZ);

    // 配置CONFIG寄存器, 禁用外部同步,DLPF带宽184Hz
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_CONFIG, MPU60X0_EXT_SYNC_DISABLED | MPU60X0_CONFIG_DLPF_184HZ);

    // 设置采样率分频寄存器,采样率 = 1kHz / (1 + SMPLRT_DIV)
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_SMPLRT_DIV, MPU60X0_SMPLRT_DIV_100Hz);

    // 配置gyro量程为±1000°/s
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_GYRO_CONFIG, MPU60X0_GYRO_CONFIG_FS_1000DPS);

    // 设置acc量程为±8g
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_ACCEL_CONFIG, MPU60X0_ACCEL_CONFIG_FS_8G);

    // 禁用ACC高通滤波器
    mpu60x0I2cReadReg8(driver->bus, MPU60X0_REG_ACCEL_CONFIG, &data[0]);
    data[0] &= ~MPU60X0_ACCEL_CONFIG_DHPF_MASK; // 清除高通滤波器位
    data[0] |= MPU60X0_ACCEL_CONFIG_DHPF_DISABLED; // 设置为禁用
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_ACCEL_CONFIG, data[0]);

    // 配置FIFO_EN寄存器,失能温度\陀螺仪\加速度数据FIFO
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_FIFO_EN, 0);

    // 配置I2C_MST_CTRL寄存器,失能多主模式和从设备FIFO
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_I2C_MST_CTRL, MPU60X0_I2C_MST_CLK_400KHZ | MPU60X0_I2C_MST_P_NSR);

    // 所有I2C从设备均失能
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_I2C_SLV0_CTRL, 0);
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_I2C_SLV1_CTRL, 0);
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_I2C_SLV2_CTRL, 0);
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_I2C_SLV3_CTRL, 0);

    // 配置自由落体检测阈值和持续时间
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_FF_THR, MPU60X0_FF_THR_0P1G);
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_FF_DUR, MPU60X0_FF_DUR_50MS);

    // 配置运动检测阈值和持续时间
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_MOT_THR, MPU60X0_MOT_THR_50MG);
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_MOT_DUR, MPU60X0_MOT_DUR_20MS);

    // 配置静止检测阈值和持续时间
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_ZMOT_THR, MPU60X0_ZMOT_THR_20MG);
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_ZMOT_DUR, MPU60X0_ZMOT_DUR_50MS);

    // 中断pin配置, ，非锁存中断、启用I2C接口、禁用FSYNC
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_INT_PIN_CFG, 0);
    // 配置中断使能寄存器，使能所有中断
    data[0] = MPU60X0_FF_INT_EN | MPU60X0_MOT_INT_EN | MPU60X0_ZMOT_INT_EN | MPU60X0_FIFO_OFLOW_INT_EN |
        MPU60X0_I2C_MST_INT_EN | MPU60X0_DATA_RDY_INT_EN;
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_INT_ENABLE, data[0]);

    // 复位FIFO、I2C主控和A+G+T通路
    data[0] = MPU60X0_FIFO_RESET | MPU60X0_I2C_MST_RESET | MPU60X0_SIG_COND_RESET;
    mpu60x0I2cWriteReg8(driver->bus, MPU60X0_REG_USER_CTRL, data[0]);

    // mpu60x0SelfTest(driver);

    mpu60x0CheckDeviceStatus(driver);
    PRINT_INFO("MPU6050 initialized successfully!");
}