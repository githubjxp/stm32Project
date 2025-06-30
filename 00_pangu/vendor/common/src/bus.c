/*
 * @file bus.h
 * @author jiangxiaoping
 * @date 2025-07-24
 * @version v1.0.0
 */

#include "bus.h"

#include "stm32f1xx.h"

void *getBusHandle(BusType_t busType, int32_t busId) {
    switch (busType) {
        case BUS_TYPE_I2C:
            return getI2cHandleByFuncID(busId);
        case BUS_TYPE_SPI:
            return getSpiHandleByFuncID(busId);
        case BUS_TYPE_UART:
            return getUartHandleByFuncID(busId);
        default:
            return NULL; // Invalid bus type
    }
}