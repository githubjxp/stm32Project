/**
 * @file
 * @author jiangxiaoping
 * @date 2025-07-07
 * @version v1.0.0
 */

 #ifndef IMU_H
 #define IMU_H

#include "stm32f1xx.h"

#include "driver.h"

#define ACC_SENSOR   0
#define GYRO_SENSOR  1
#define IMU_SENSOR_COUNT 2

#define AXIS_COUNT 3
#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

typedef struct {
    int revolution[IMU_SENSOR_COUNT]; // 0: acc, 1: gyro
    int accOffset[AXIS_COUNT];
    int gyroOffset[AXIS_COUNT];
} imu_params_t;

 #endif // IMU_H