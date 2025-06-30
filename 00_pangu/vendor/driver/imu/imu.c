/**
 * @file
 * @author jiangxiaoping
 * @date 2025-07-07
 * @version v1.0.0
 */

#include "imu.h"

#include "bus.h"
#include "driver.h"
#include "tag.h"
#include "print.h"

#include "mpu60x0.h"
#include "hw_interface.h"

static imu_params_t g_ImuParams = {0};

int getImuResolution(Driver_t *driver, uint8_t type)
{
    float resolution[1];
    switch (type) {
        case ACC_SENSOR:
            mpu60x0GetAccResolution(driver, resolution);
            return resolution[0];
        case GYRO_SENSOR:
            mpu60x0GetGyroResolution(driver, resolution);
            return resolution[0];
        default:
            return 0;
    }
}

int32_t imuConfig(Driver_t *driver, void *config)
{
    if (driver == NULL || config == NULL) {
        PRINT_ERROR("Invalid parameters!");
        return ERROR;
    }

    PRINT_INFO("IMU configured successfully!");
    return SUCCESS;
}

int32_t imuRead(Driver_t *driver, uint8_t *data, uint32_t size)
{
    if (driver == NULL || data == NULL || size < sizeof(uint32_t) * AXIS_COUNT * 2) {
        PRINT_ERROR("Invalid parameters!");
        return ERROR;
    }

    // 读取IMU数据
    int16_t acc_data[AXIS_COUNT] = {0};
    int16_t gyro_data[AXIS_COUNT] = {0};

    if (mpu60x0ReadAcc(driver, acc_data, AXIS_COUNT) != SUCCESS ||
        mpu60x0ReadGyro(driver, gyro_data, AXIS_COUNT) != SUCCESS) {
        PRINT_ERROR("Failed to read IMU data!");
        return ERROR;
    }

    // 将数据填充到输出缓冲区
    for (uint32_t i = 0; i < AXIS_COUNT; i++) {
        ((int32_t *)data)[i] = acc_data[i] * g_ImuParams.revolution[ACC_SENSOR] * 98 / 10000 + g_ImuParams.accOffset[i];
        ((int32_t *)data)[i + AXIS_COUNT] =
            gyro_data[i] * g_ImuParams.revolution[GYRO_SENSOR] / 1000 + g_ImuParams.gyroOffset[i];
    }

    PRINT_INFO("IMU data read successfully!");
    return SUCCESS;
}

int32_t imuClose(Driver_t *driver)
{
    if (driver == NULL) {
        PRINT_ERROR("Driver is NULL!");
        return ERROR;
    }

    PRINT_INFO("IMU closed successfully!");
    return SUCCESS;
}

int32_t imuOpen(Driver_t *driver)
{
    if (driver == NULL) {
        PRINT_ERROR("Driver is NULL!");
        return ERROR;
    }

    PRINT_INFO("IMU opened successfully!");
    return SUCCESS;
}

int32_t imuInit(Driver_t *driver)
{
    if (driver == NULL) {
        PRINT_ERROR("Driver is NULL!");
        return ERROR;
    }

    // 获取总线句柄
    driver->bus.handle = getBusHandle(driver->bus.type, driver->bus.id);
    if (driver->bus.handle == NULL) {
        PRINT_ERROR("Failed to get bus handle!");
        return ERROR;
    }

    mpu60x0Init(driver);

    // 配置imu参数
    driver->params = &g_ImuParams;
    g_ImuParams.revolution[ACC_SENSOR] = getImuResolution(driver, ACC_SENSOR);
    g_ImuParams.revolution[GYRO_SENSOR] = getImuResolution(driver, GYRO_SENSOR);
    g_ImuParams.accOffset[X_AXIS] = -575;
    g_ImuParams.accOffset[Y_AXIS] = -305;
    g_ImuParams.accOffset[Z_AXIS] = 370;
    g_ImuParams.gyroOffset[X_AXIS] = 5;
    g_ImuParams.gyroOffset[Y_AXIS] = 0;
    g_ImuParams.gyroOffset[Z_AXIS] = 0;

    PRINT_INFO("IMU initialized successfully!");
    return SUCCESS;
}

DriverOps_t g_imuOps = {
    .init = imuInit,
    .open = imuOpen,
    .close = imuClose,
    .read = imuRead,
    .config = imuConfig,
    .setInterval = NULL, // IMU driver set interval function will be defined later
};

Driver_t g_imuDriver = {
    .tag = DRIVER_TAG_IMU,
    .ops = &g_imuOps,
    .driverData = NULL,
    .driverDataLen = 0,
    .priv = NULL, // Private data for the IMU driver
    .bus = {
        .type = BUS_TYPE_I2C, // Assuming IMU uses I2C bus
        .id = I2C_FUNC_ID_MPU6050, // Using I2C bus 0
        .handle = NULL, // This will be set to the actual I2C handle later
    }
};

void imuDriverRegister(void)
{
    // Register the IMU driver
    driverRegister(&g_imuDriver);
    PRINT_INFO("IMU driver registered successfully!");
}

DRIVER_INIT(imuDriverRegister)