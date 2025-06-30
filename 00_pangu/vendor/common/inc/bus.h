/*
 * @file bus.h
 * @author jiangxiaoping
 * @date 2025-07-24
 * @version v1.0.0
 */

#ifndef _BUS_H
#define _BUS_H

#include "i2c_config.h"
#include "spi_config.h"
#include "uart_config.h"

#include "stm32f1xx.h"

typedef struct bus_t {
    uint8_t type; // 0: I2C, 1: SPI, 2: UART
    int32_t id; // 对应各个总线的funcId, 通过type + id获取bus handle
    void    *handle; // Private data for the bus, e.g., I2C handle, SPI handle, UART handle
} bus_t;

typedef enum {
    BUS_TYPE_I2C = 0,
    BUS_TYPE_SPI,
    BUS_TYPE_UART,
} BusType_t;

#define i2cRead(bus, addr, data, len) \
    HAL_I2C_Master_Receive((I2C_HandleTypeDef *)(bus).handle, (addr), (data), (len), HAL_MAX_DELAY)
#define i2cWrite(bus, addr, data, len) \
    HAL_I2C_Master_Transmit((I2C_HandleTypeDef *)(bus).handle, (addr), (data), (len), HAL_MAX_DELAY)
#define i2cReadReg8(bus, addr, reg, data, len) HAL_I2C_Mem_Read((I2C_HandleTypeDef *)(bus).handle, (addr), \
    (reg), I2C_MEMADD_SIZE_8BIT, (data), (len), HAL_MAX_DELAY)
#define i2cWriteReg8(bus, addr, reg, data, len) HAL_I2C_Mem_Write((I2C_HandleTypeDef *)(bus).handle, (addr), \
    (reg), I2C_MEMADD_SIZE_8BIT, (data), (len), HAL_MAX_DELAY)
#define i2cReadReg16(bus, addr, reg, data, len) HAL_I2C_Mem_Read((I2C_HandleTypeDef *)(bus).handle, (addr), \
    (reg), I2C_MEMADD_SIZE_16BIT, (data), (len), HAL_MAX_DELAY)
#define i2cWriteReg16(bus, addr, reg, data, len) HAL_I2C_Mem_Write((I2C_HandleTypeDef *)(bus).handle, (addr), \
    (reg), I2C_MEMADD_SIZE_16BIT, (data), (len), HAL_MAX_DELAY)

#define i2cDmaRead(bus, addr, data, len) \
    HAL_I2C_Master_Receive_DMA((I2C_HandleTypeDef *)(bus).handle, (addr), (data), (len))
#define i2cDmaWrite(bus, addr, data, len) \
    HAL_I2C_Master_Transmit_DMA((I2C_HandleTypeDef *)(bus).handle, (addr), (data), (len))
#define i2cDmaReadReg8(bus, addr, reg, data, len) \
    HAL_I2C_Mem_Read_DMA((I2C_HandleTypeDef *)(bus).handle, (addr), (reg), I2C_MEMADD_SIZE_8BIT, (data), (len))
#define i2cDmaWriteReg8(bus, addr, reg, data, len) \
    HAL_I2C_Mem_Write_DMA((I2C_HandleTypeDef *)(bus).handle, (addr), (reg), I2C_MEMADD_SIZE_8BIT, (data), (len))
#define i2cDmaReadReg16(bus, addr, reg, data, len) \
    HAL_I2C_Mem_Read_DMA((I2C_HandleTypeDef *)(bus).handle, (addr), (reg), I2C_MEMADD_SIZE_16BIT, (data), (len))
#define i2cDmaWriteReg16(bus, addr, reg, data, len) \
    HAL_I2C_Mem_Write_DMA((I2C_HandleTypeDef *)(bus).handle, (addr), (reg), I2C_MEMADD_SIZE_16BIT, (data), (len))


#define spiRead(bus, data, len) \
    HAL_SPI_Receive((SPI_HandleTypeDef *)(bus).handle, (data), (len), HAL_MAX_DELAY)
#define spiWrite(bus, data, len) \
    HAL_SPI_Transmit((SPI_HandleTypeDef *)(bus).handle, (data), (len), HAL_MAX_DELAY)
#define i2cDmaRead(bus, addr, data, len) \
    HAL_I2C_Master_Receive_DMA((I2C_HandleTypeDef *)(bus).handle, (addr), (data), (len))
#define i2cDmaWrite(bus, addr, data, len) \
    HAL_I2C_Master_Transmit_DMA((I2C_HandleTypeDef *)(bus).handle, (addr), (data), (len))

#define uartRead(bus, data, len) \
    HAL_UART_Receive((UART_HandleTypeDef *)(bus).handle, (data), (len), HAL_MAX_DELAY)
#define uartWrite(bus, data, len) \
    HAL_UART_Transmit((UART_HandleTypeDef *)(bus).handle, (data), (len), HAL_MAX_DELAY)
#define uartDmaRead(bus, data, len) \
    HAL_UART_Receive_DMA((UART_HandleTypeDef *)(bus).handle, (data), (len))
#define uartDmaWrite(bus, data, len) \
    HAL_UART_Transmit_DMA((UART_HandleTypeDef *)(bus).handle, (data), (len))

void *getBusHandle(BusType_t busType, int32_t busId);
#endif // _BUS_H