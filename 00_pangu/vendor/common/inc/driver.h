/**
 * @file
 * @author jiangxiaoping
 * @date 2025-07-09
 * @version v1.0.0
 */

#ifndef _DRIVER_H
#define _DRIVER_H

#include "bus.h"

#include <stdint.h>

extern struct Driver_t;
typedef struct Driver_t Driver_t; 

typedef struct DriverOps_t {
    int32_t (*init)(Driver_t *driver);
    int32_t (*open)(Driver_t *driver);
    int32_t (*close)(Driver_t *driver);
    int32_t (*read)(Driver_t *driver, uint8_t *data, uint32_t len);
    int32_t (*config)(Driver_t *driver, void *config);
    int32_t (*setInterval)(Driver_t *driver, uint32_t interval);
} DriverOps_t;

struct Driver_t {
    uint32_t tag;
    DriverOps_t *ops;
    uint8_t *driverData;
    uint32_t driverDataLen;
    bus_t bus;
    void *params; // 驱动特征参数，比如IMU设备中，param可以是陀螺仪和加速度计的量程、分辨率等
    void *priv;
};

typedef void (*DriverInitFunc_t)(void);
#define DRIVER_INIT(func) __attribute__((section(".driver_init"), used)) DriverInitFunc_t func##_init = func;

Driver_t *getDriverByTag(const uint32_t tag);
void driverRegister(Driver_t *driver);
void runDriverInit(void);

#endif // _DRIVER_H