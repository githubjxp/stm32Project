/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-16
 * @version v1.0.0
 */

#include "user_task.h"

#include "i2c_config.h"
#include "imu.h"
#include "led.h"
#include "spi_config.h"
#include "tag.h"
#include "print.h"
#include "uart_config.h"
#include "uart_it.h"

#include "stm32f1xx.h"
#include "FreeRTOS.h"
#include "task.h"

TaskHandle_t g_ledTaskHandle = NULL;
TaskHandle_t g_cmdTaskHandle = NULL;
TaskHandle_t g_i2cTaskHandle = NULL;
TaskHandle_t g_spiTaskHandle = NULL;

void ledTask(void *param)
{
    (void)param;

    while (1) {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5); // Toggle LED0
        // PRINT_INFO("LED0 toggled");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static uint8_t *getUartProcessData(UartBuff_t *uartBuff)
{
    uint8_t *data = (uint8_t *)pvPortMalloc(uartBuff->size);
    if (data == NULL) {
        PRINT_ERROR("Failed to allocate memory for UART data processing");
        return NULL;
    }
    memset(data, 0, uartBuff->size);
    memcpy(data, uartBuff->buff, uartBuff->pos);
    data[uartBuff->pos] = '\0';
    uartBuff->pos = 0;

    return data;
}

void cmdTask(void *param)
{
    (void)param;

    while (1) {
        PRINT_INFO("***************Command task running***************");
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Wait for notification
        vTaskDelay(pdMS_TO_TICKS(1000));
        UartConfig_t *uartCfg = getUartConfigByFuncID(UART_FUNC_ID_PC_COMMUNICATION);
        uint8_t *uartData = NULL;

        if (uartCfg->rxBuffState == UART_BUFF_STATE_BUFF0_RECEIVING) {
            if (uartCfg->rxBuff1.pos > 0) {
                uartData = getUartProcessData(&uartCfg->rxBuff1); // 0 recv 1 process
            } else if (uartCfg->rxBuff0.pos > 0) { // 走到该逻辑说明上一帧数据处理过慢，连续接收了两次Notify
                uartBuffSwitch(uartCfg); // 切换BUFF1接收
                uartData = getUartProcessData(&uartCfg->rxBuff0); // 处理BUFF0数据
            }
        } else if (uartCfg->rxBuffState == UART_BUFF_STATE_BUFF1_RECEIVING) {
            if (uartCfg->rxBuff0.pos > 0) {
                uartData = getUartProcessData(&uartCfg->rxBuff0); // 1 recv 0 process
            } else if (uartCfg->rxBuff1.pos > 0) { // 走到该逻辑说明上一帧数据处理过慢，连续接收了两次Notify
                uartBuffSwitch(uartCfg); // 切换BUFF0接收
                uartData = getUartProcessData(&uartCfg->rxBuff1); // 处理BUFF1数据
            }
        }

        if (uartData != NULL) {
            PRINT_INFO("Received data: %s", uartData);
        }
        
        PRINT_INFO("buff size: %d %d, buff0 pos: %d, buff1 pos: %d, state: %d", uartCfg->rxBuff0.size,
            uartCfg->rxBuff1.size, uartCfg->rxBuff0.pos, uartCfg->rxBuff1.pos, uartCfg->rxBuffState);
        vPortFree(uartData);
    }
}

/*
void i2cTask(void *param)
{
    (void)param;

    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET); // Set I2C pins high (SCL and SDA)

    I2C_HandleTypeDef *i2cHandle = getI2cHandleByFuncID(I2C_FUNC_ID_MPU6050);
    if (i2cHandle == NULL) {
        PRINT_ERROR("I2C handle not found for function ID: %d", I2C_FUNC_ID_MPU6050);
        vTaskDelete(NULL); // Delete the task if I2C handle is not found
        return;
    }

    uint8_t memAddr = 0x01;
    int32_t ret;
    uint8_t wdata[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    while (1) {
        PRINT_INFO("I2C task running");
        uint8_t rdata[16] = {0};

        // ret = HAL_I2C_Mem_Write(i2cHandle, I2C1_ADDR_YWDZ, memAddr, I2C_MEMADD_SIZE_8BIT, wdata + memAddr, 1, 0xFF);
        ret = HAL_I2C_Mem_Read(i2cHandle, I2C1_ADDR_YWDZ, memAddr, I2C_MEMADD_SIZE_8BIT, rdata, sizeof(rdata), 0xFF);

        // ret = HAL_I2C_Master_Transmit(i2cHandle, I2C1_ADDR_YWDZ, wdata + memAddr, 1, 0xFF);
        // ret = HAL_I2C_Master_Receive(i2cHandle, I2C1_ADDR_YWDZ, rdata, sizeof(rdata), 0xFF);
        // PRINT_INFO("I2C read data: %02X %02X, %d", memAddr, rdata[0], ret);

        // for (int i = 0; i < sizeof(rdata); i++)
        //     PRINT_INFO("I2C read data[%d]: %02X", i, rdata[i]);

        // memAddr = (memAddr + 1) % 16; // Increment memory address for next read
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
*/

void getImuData(void *param)
{
    (void)param;

    Driver_t *imuDriver = getDriverByTag(DRIVER_TAG_IMU);
    if (imuDriver == NULL) {
        PRINT_ERROR("IMU driver not found");
        vTaskDelete(NULL); // Delete the task if IMU driver is not found
        return;
    }

    int32_t imu_data[AXIS_COUNT * 2] = {0};
    int ret;

    while (1) {
        ret = imuDriver->ops->read(imuDriver, (uint8_t *)imu_data, sizeof(imu_data));
        if (ret != SUCCESS) {
            PRINT_ERROR("Failed to read IMU data");
        } else {
            PRINT_INFO("IMU Data: accX=%d, accY=%d, accZ=%d, gyroX=%d, gyroY=%d, gyroZ=%d",
                imu_data[0], imu_data[1], imu_data[2], imu_data[3], imu_data[4], imu_data[5]);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void spiTask(void *param)
{
    (void)param;

    SPI_HandleTypeDef *spiHandle = getSpiHandleByFuncID(SPI_FUNC_ID_FLASH_W25Q128);
    if (spiHandle == NULL) {
        PRINT_ERROR("SPI handle not found for function ID: %d", SPI_FUNC_ID_FLASH_W25Q128);
        vTaskDelete(NULL); // Delete the task if SPI handle is not found
        return;
    }

    int ret;

    // SPI task implementation goes here
    while (1) {
        PRINT_INFO("SPI task running");
        uint8_t wdata[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        uint8_t rdata[4] = {0};

        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8, GPIO_PIN_RESET);
        ret = HAL_SPI_Transmit(spiHandle, wdata, sizeof(wdata), HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            PRINT_ERROR("SPI transmit failed: %d", ret);
        } else {
            PRINT_INFO("SPI transmit successful");
        }

        ret = HAL_SPI_Receive(spiHandle, rdata, sizeof(rdata), HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            PRINT_ERROR("SPI receive failed: %d", ret);
        } else {
            PRINT_INFO("SPI receive successful");
            PRINT_INFO("SPI received data:%02X, %02X, %02X, %02X", rdata[0], rdata[1], rdata[2], rdata[3]);
        }

        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8, GPIO_PIN_SET);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void taskEnter(void)
{
    PRINT_INFO("taskEnter");
    int32_t ret;

    ret = xTaskCreate(ledTask, LED_TASK_NAME, LED_TASK_STACK_SIZE, NULL, LED_TASK_PRIORITY, NULL);
    if (ret != pdPASS) {
        PRINT_WARN("ledTask create failed");
    }

    ret = xTaskCreate(cmdTask, CMD_TASK_NAME, CMD_TASK_STACK_SIZE, NULL, CMD_TASK_PRIORITY, &g_cmdTaskHandle);
    if (ret != pdPASS) {
        PRINT_WARN("cmdTask create failed");
    }

    ret = xTaskCreate(getImuData, I2C_TASK_NAME, I2C_TASK_STACK_SIZE, NULL, I2C_TASK_PRIORITY, &g_i2cTaskHandle);
    if (ret != pdPASS) {
        PRINT_WARN("i2cTask create failed");
    }

    ret = xTaskCreate(spiTask, SPI_TASK_NAME, SPI_TASK_STACK_SIZE, NULL, SPI_TASK_PRIORITY, &g_spiTaskHandle);
    if (ret != pdPASS) {
        PRINT_WARN("spiTask create failed");
    }

    PRINT_INFO("Scheduler started");
    vTaskStartScheduler();
}
