/**
 * @file mpu60x0.h
 * @author jiangxiaoping
 * @date 2025-07-07
 * @version v1.0.0
 */

 /*
 * a+g数据输出方式
 * FIFO：一个先入先出的数据缓冲区，用于暂存数据供主处理器稍后读取。
 * DMP：数字运动处理器模块，用于内部处理这些数据。
 * Sensor Registers：存储传感器最新读数以供主处理器直接读取的寄存器
 */

/*
 * DLPF作用：数字低通滤波器，用于减少传感器数据中的高频噪声
 * 工程建议：在满足信噪比前提下，尽可能选择高带宽配置。逐步增加DLPF带宽直到噪声明显增大，回退一档作为最优值
 */

/*
 * 注意：如果ACCEL_HPF使能，三轴加速度计数据将被高通滤波处理，重力分量将被滤除，即为0g，而非1g，也就意味着失重无法通过0g检测
 * ACCEL_HPF: 数字高通滤波器，用于去除加速度计数据中的低频漂移
 * ACCEL_HPF 的核心价值：通过滤除重力等低频干扰，提升运动检测的准确性。实际配置需结合应用场景：
 * 静态监测（如姿态估计）：关闭滤波器（ACCEL_HPF=000），保留重力分量8。
 * 动态检测（如自由落体）：启用滤波（如 ACCEL_HPF=001）并设置合适截止频率69。
 * 瞬时事件捕捉：使用持续模式（ACCEL_HPF=010）锁定基准值6。
 */

/*
 * FSYNC作用： 将外部设备通过FSYNC引脚发送的同步信号嵌入传感器数据流，用于多设备时序对齐
 * 典型场景：在需要多传感器同步的场景（如无人机姿态融合），通过FSYNC信号触发所有传感器同时采样，确保数据时间戳一致
 */

/*
 * FIFO注意事项：
 * 1. FIFO缓冲区的最大深度为1024字节，超过后将覆盖最早数据
 * 2. 使用运动检测功能时，需关闭FIFO, 以避免数据冲突
 * 3. FIFO典型数据流顺序：加速度 → 温度 → 陀螺仪 → 辅助传感器（若使能）
 */

/*
 * I2C从设备功能：读取外部传感器数据（如磁力计、气压计等）或写入配置
 * I2C从设备0-3 和 4的区别
 * 特性	       Slave 0-3	                    Slave 4
 * 数据传输量	   最多15字节	                     单字节
 * 数据传输方式   自动存储到EXT_SENS_DATA	         通过专用DI/DO寄存器
 * 自动递增	   支持(REG_DIS位)	                不支持
 * 连续传输	   支持	                            每次操作需重新触发
 * 主要用途	   批量数据采集	                     设备配置/寄存器访问
 * 传输触发	   设置CTRL寄存器即开始	             需设置EN位触发
 * 中断支持	   有限	                            专用中断位(INT_EN)
 * 字节交换       支持(BYTE_SW位)	                 不支持
 * 分组存储       支持(GRP位)	                     不支持
 */

/*
 * I2C从设备相关寄存器，以slave 1为例：
 * 寄存器名称	      地址	    功能
 * I2C_SLV1_ADDR     0x25	   外接从设备的I²C地址（含读写位）	0x3C（写操作）
 * I2C_SLV1_REG      0x26	   目标从设备的寄存器地址	0x00（配置寄存器）
 * I2C_SLV1_DO       0x65	   待写入的数据字节	0x01（连续模式）
 * I2C_SLV1_CTRL     0x27	   控制位（使能、读写、数据长度）
 */

/*
 * MPU60X0 时钟源选择对比：
 * 时钟源	        10分钟角度漂移(z轴角度，航向角)	     功耗
 * 8MHz RC振荡器	8.7°	                           3.9mA
 * X轴陀螺仪+PLL	0.9°	                           0.4mA
 * 外部19.2MHz晶振	0.2°	                           5.2mA
 */

#ifndef _MPU60X0_H
#define _MPU60X0_H

#include "bus.h"
#include "driver.h"

#include "stm32f1xx.h"

#include "stdbool.h"

#define MPU6050_I2C_ADDRESS             (0x68 << 1) // B110100X, X为AD0的电平，低电平地址为0x68，高电平地址为0x69
#define MPU6050_MAX_FIFO_SIZE           1024 // unit: bytes, 在特性介绍里说明了

#define MPU60X0_REG_AUX_VDDIO           0x01 // 输出电压
#define MPU60X0_REG_SMPLRT_DIV          0x19 // 采样率分频，采样率 = 1kHz / (1 + SMPLRT_DIV)
#define MPU60X0_REG_CONFIG              0x1A // 配置数字低通滤波器（DLPF）和外部同步采样（FSYNC引脚功能）
#define MPU60X0_REG_GYRO_CONFIG         0x1B // 配置陀螺仪的三轴自检和量程，陀螺仪自检可分别和同时进行
#define MPU60X0_REG_ACCEL_CONFIG        0x1C // 配置加速度计的三轴自检和量程，以及acc的DHPF(数字高通滤波器)
#define MPU60X0_REG_FF_THR              0x1D // 自由落体阈值, 三轴合加速度小于该阈值，开始计时 1LSB = 2mg
#define MPU60X0_REG_FF_DUR              0x1E // 自由落体持续时间 1LSB = 1ms
#define MPU60X0_REG_MOT_THR             0x1F // 运动阈值, 任意轴加速度超过该阈值，开始计时 1LSB = 2mg
#define MPU60X0_REG_MOT_DUR             0x20 // 运动持续时间  1LSB = 1ms
#define MPU60X0_REG_ZMOT_THR            0x21 // zero motion，静止阈值, 三轴合加速度小于该阈值，开始计时 1LSB = 2mg
#define MPU60X0_REG_ZMOT_DUR            0x22 // zero motion，持续时间 1LSB = 1ms
#define MPU60X0_REG_FIFO_EN             0x23 // FIFO使能寄存器，用于配置FIFO缓冲区的内容, gyro三轴单独配置，acc统一配置
#define MPU60X0_REG_I2C_MST_CTRL        0x24 // I2C主设备控制寄存器，用于配置I2C主设备的工作模式和时序, 比如用于磁力计等外设的I2C通信
#define MPU60X0_REG_I2C_SLV0_ADDR       0x25 // MPU60X0的I2C从设备0地址寄存器，用于配置I2C从设备的地址
#define MPU60X0_REG_I2C_SLV0_REG        0x26 // MPU60X0的I2C从设备0寄存器，用于配置I2C从设备的寄存器
#define MPU60X0_REG_I2C_SLV0_CTRL       0x27 // MPU60X0的I2C从设备0控制寄存器，用于配置I2C从设备的控制参数
#define MPU60X0_REG_I2C_SLV1_ADDR       0x28 // MPU60X0的I2C从设备1地址寄存器，用于配置I2C从设备的地址
#define MPU60X0_REG_I2C_SLV1_REG        0x29 // MPU60X0的I2C从设备1寄存器，用于配置I2C从设备的寄存器
#define MPU60X0_REG_I2C_SLV1_CTRL       0x2A // MPU60X0的I2C从设备1控制寄存器，用于配置I2C从设备的控制参数
#define MPU60X0_REG_I2C_SLV2_ADDR       0x2B // MPU60X0的I2C从设备2地址寄存器，用于配置I2C从设备的地址
#define MPU60X0_REG_I2C_SLV2_REG        0x2C // MPU60X0的I2C从设备2寄存器，用于配置I2C从设备的寄存器
#define MPU60X0_REG_I2C_SLV2_CTRL       0x2D // MPU60X0的I2C从设备2控制寄存器，用于配置I2C从设备的控制参数
#define MPU60X0_REG_I2C_SLV3_ADDR       0x2E // MPU60X0的I2C从设备3地址寄存器，用于配置I2C从设备的地址
#define MPU60X0_REG_I2C_SLV3_REG        0x2F // MPU60X0的I2C从设备3寄存器，用于配置I2C从设备的寄存器
#define MPU60X0_REG_I2C_SLV3_CTRL       0x30 // MPU60X0的I2C从设备3控制寄存器，用于配置I2C从设备的控制参数
#define MPU60X0_REG_I2C_SLV4_ADDR       0x31 // MPU60X0的I2C从设备4地址寄存器，用于配置I2C从设备的地址
#define MPU60X0_REG_I2C_SLV4_REG        0x32 // MPU60X0的I2C从设备4寄存器，用于配置I2C从设备的寄存器
#define MPU60X0_REG_I2C_SLV4_DO         0x33 // MPU60X0的I2C从设备4数据输出寄存器，用于配置I2C从设备的数据输出
#define MPU60X0_REG_I2C_SLV4_CTRL       0x34 // MPU60X0的I2C从设备4控制寄存器，用于配置I2C从设备的控制参数
#define MPU60X0_REG_I2C_SLV4_DI         0x35 // MPU60X0的I2C从设备4数据输入寄存器，用于配置I2C从设备的数据输入
#define MPU60X0_REG_I2C_MST_STATUS      0x36 // I2C主设备状态寄存器，用于读取I2C主设备的状态
#define MPU60X0_REG_INT_PIN_CFG         0x37 // 中断引脚配置寄存器，用于配置中断引脚的工作模式和功能
#define MPU60X0_REG_INT_ENABLE          0x38 // 中断使能寄存器，用于使能或禁用中断
#define MPU60X0_REG_INT_STATUS          0x3A // 中断状态寄存器，用于读取中断状态
#define MPU60X0_REG_ACCEL_XOUT_H        0x3B // 加速度计X轴输出高字节
#define MPU60X0_REG_ACCEL_XOUT_L        0x3C // 加速度计X轴输出低字节
#define MPU60X0_REG_ACCEL_YOUT_H        0x3D // 加速度计Y轴输出高字节
#define MPU60X0_REG_ACCEL_YOUT_L        0x3E // 加速度计Y轴输出低字节
#define MPU60X0_REG_ACCEL_ZOUT_H        0x3F // 加速度计Z轴输出高字节
#define MPU60X0_REG_ACCEL_ZOUT_L        0x40 // 加速度计Z轴输出低字节
#define MPU60X0_REG_TEMP_OUT_H          0x41 // 温度传感器输出高字节
#define MPU60X0_REG_TEMP_OUT_L          0x42 // 温度传感器输出低字节
#define MPU60X0_REG_GYRO_XOUT_H         0x43 // 陀螺仪X轴输出高字节
#define MPU60X0_REG_GYRO_XOUT_L         0x44 // 陀螺仪X轴输出低字节
#define MPU60X0_REG_GYRO_YOUT_H         0x45 // 陀螺仪Y轴输出高字节
#define MPU60X0_REG_GYRO_YOUT_L         0x46 // 陀螺仪Y轴输出低字节
#define MPU60X0_REG_GYRO_ZOUT_H         0x47 // 陀螺仪Z轴输出高字节
#define MPU60X0_REG_GYRO_ZOUT_L         0x48 // 陀螺仪Z轴输出低字节
#define MPU60X0_REG_EXT_SENS_DATA_00    0x49 // 外部传感器数据0
#define MPU60X0_REG_EXT_SENS_DATA_01    0x4A // 外部传感器数据1
#define MPU60X0_REG_EXT_SENS_DATA_02    0x4B // 外部传感器数据2
#define MPU60X0_REG_EXT_SENS_DATA_03    0x4C // 外部传感器数据3
#define MPU60X0_REG_EXT_SENS_DATA_04    0x4D // 外部传感器数据4
#define MPU60X0_REG_EXT_SENS_DATA_05    0x4E // 外部传感器数据5
#define MPU60X0_REG_EXT_SENS_DATA_06    0x4F // 外部传感器数据6
#define MPU60X0_REG_EXT_SENS_DATA_07    0x50 // 外部传感器数据7
#define MPU60X0_REG_EXT_SENS_DATA_08    0x51 // 外部传感器数据8
#define MPU60X0_REG_EXT_SENS_DATA_09    0x52 // 外部传感器数据9
#define MPU60X0_REG_EXT_SENS_DATA_10    0x53 // 外部传感器数据10
#define MPU60X0_REG_EXT_SENS_DATA_11    0x54 // 外部传感器数据11
#define MPU60X0_REG_EXT_SENS_DATA_12    0x55 // 外部传感器数据12
#define MPU60X0_REG_EXT_SENS_DATA_13    0x56 // 外部传感器数据13
#define MPU60X0_REG_EXT_SENS_DATA_14    0x57 // 外部传感器数据14
#define MPU60X0_REG_EXT_SENS_DATA_15    0x58 // 外部传感器数据15
#define MPU60X0_REG_EXT_SENS_DATA_16    0x59 // 外部传感器数据16
#define MPU60X0_REG_EXT_SENS_DATA_17    0x5A // 外部传感器数据17
#define MPU60X0_REG_EXT_SENS_DATA_18    0x5B // 外部传感器数据18
#define MPU60X0_REG_EXT_SENS_DATA_19    0x5C // 外部传感器数据19
#define MPU60X0_REG_EXT_SENS_DATA_20    0x5D // 外部传感器数据20
#define MPU60X0_REG_EXT_SENS_DATA_21    0x5E // 外部传感器数据21
#define MPU60X0_REG_EXT_SENS_DATA_22    0x5F // 外部传感器数据22
#define MPU60X0_REG_EXT_SENS_DATA_23    0x60 // 外部传感器数据23
#define MPU60X0_REG_MOT_DETECT_STATUS   0x61 // 运动检测状态
#define MPU60X0_REG_I2C_SLV0_DO         0x63 // I2C从设备0数据输出寄存器
#define MPU60X0_REG_I2C_SLV1_DO         0x64 // I2C从设备1数据输出寄存器
#define MPU60X0_REG_I2C_SLV2_DO         0x65 // I2C从设备2数据输出寄存器
#define MPU60X0_REG_I2C_SLV3_DO         0x66 // I2C从设备3数据输出寄存器
#define MPU60X0_REG_I2C_MST_DELAY_CTRL  0x67 // I2C主设备延迟控制寄存器
#define MPU60X0_REG_SIGNAL_PATH_RESET   0x68 // 信号路径复位寄存器，用于复位陀螺仪、加速度计和温度传感器的数据路径
#define MPU60X0_REG_MOT_DETECT_CTRL     0x69 // 运动检测控制寄存器，用于配置运动检测的阈值和持续时间等参数
#define MPU60X0_REG_USER_CTRL           0x6A // 用户控制寄存器，用于配置MPU60X0的工作模式和功能
#define MPU60X0_REG_PWR_MGMT_1          0x6B // 电源管理1寄存器，用于配置MPU60X0的电源管理功能
#define MPU60X0_REG_PWR_MGMT_2          0x6C // 电源管理2寄存器，用于配置MPU60X0的电源管理功能
#define MPU60X0_REG_FIFO_COUNTH         0x72 // FIFO数据计数高字节
#define MPU60X0_REG_FIFO_COUNTL         0x73 // FIFO数据计数低字节
#define MPU60X0_REG_FIFO_R_W            0x74 // FIFO读写寄存器
#define MPU60X0_REG_WHO_AM_I            0x75

// MPU60X0_REG_AUX_VDDIO
#define AUX_VDDIO_VDD                  (0x01 << 7)
#define AUX_VDDIO_VLOGIC               (0x00 << 7)

// MPU60X0_REG_SMPLRT_DIV
#define MPU60X0_SMPLRT_DIV_1KHz        0x00 // 1kHz采样率
#define MPU60X0_SMPLRT_DIV_500Hz       0x01 // 500Hz采样率
#define MPU60X0_SMPLRT_DIV_250Hz       0x03 // 250Hz采样率
#define MPU60X0_SMPLRT_DIV_125Hz       0x07 // 125Hz采样率
#define MPU60X0_SMPLRT_DIV_100Hz       0x09 // 100Hz采样率

// MPU60X0_REG_CONFIG
#define MPU60X0_CONFIG_EXT_SYNC_MASK   (0x07 << 3) // 外部同步采样配置掩码
#define MPU60X0_EXT_SYNC_DISABLED      (0x00 << 3) // 禁用外部同步
#define MPU60X0_EXT_SYNC_TEMP_OUT_L    (0x01 << 3) // 同步温度传感器输出
#define MPU60X0_EXT_SYNC_GYRO_XOUT_L   (0x02 << 3) // 同步陀螺仪X轴输出
#define MPU60X0_EXT_SYNC_GYRO_YOUT_L   (0x03 << 3) // 同步陀螺仪Y轴输出
#define MPU60X0_EXT_SYNC_GYRO_ZOUT_L   (0x04 << 3) // 同步陀螺仪Z轴输出
#define MPU60X0_EXT_SYNC_ACCEL_XOUT_L  (0x05 << 3) // 同步加速度计X轴输出
#define MPU60X0_EXT_SYNC_ACCEL_YOUT_L  (0x06 << 3) // 同步加速度计Y轴输出
#define MPU60X0_EXT_SYNC_ACCEL_ZOUT_L  (0x07 << 3) // 同步加速度计Z轴输出

#define MPU60X0_CONFIG_DLPF_MASK       (0x07 << 0) // DLPF带宽配置掩码
#define MPU60X0_CONFIG_DLPF_260HZ      (0x00 << 0) // DLPF带宽260Hz, 延迟0ms, 适用于高动态场景
#define MPU60X0_CONFIG_DLPF_184HZ      (0x01 << 0) // DLPF带宽184Hz, 延迟2ms, 适用于中等动态场景
#define MPU60X0_CONFIG_DLPF_94HZ       (0x02 << 0) // DLPF带宽94Hz, 延迟3ms, 适用于低动态场景
#define MPU60X0_CONFIG_DLPF_44HZ       (0x03 << 0) // DLPF带宽44Hz, 延迟4.9ms, 适用于非常低动态场景
#define MPU60X0_CONFIG_DLPF_21HZ       (0x04 << 0) // DLPF带宽21Hz, 延迟8.5ms, 适用于极低动态场景
#define MPU60X0_CONFIG_DLPF_10HZ       (0x05 << 0) // DLPF带宽10Hz, 延迟13.8ms, 适用于超低动态场景
#define MPU60X0_CONFIG_DLPF_5HZ        (0x06 << 0) // DLPF带宽5Hz, 延迟19.0ms, 适用于超低动态场景

// MPU60X0_REG_GYRO_CONFIG
#define MPU60X0_XGYRO_SELF_TEST  (0x01 << 7) // 陀螺仪自检使能位
#define MPU60X0_YGYRO_SELF_TEST  (0x01 << 6) // 陀螺仪自检使能位
#define MPU60X0_ZGYRO_SELF_TEST  (0x01 << 5) // 陀螺仪自检使能位

#define MPU60X0_GYRO_CONFIG_FS_MASK    (0x03 << 3) // 陀螺仪量程配置掩码
#define MPU60X0_GYRO_CONFIG_FS_250DPS  (0x00 << 3) // 陀螺仪量程±250°/s
#define MPU60X0_GYRO_CONFIG_FS_500DPS  (0x01 << 3) // 陀螺仪量程±500°/s
#define MPU60X0_GYRO_CONFIG_FS_1000DPS (0x02 << 3) // 陀螺仪量程±1000°/s
#define MPU60X0_GYRO_CONFIG_FS_2000DPS (0x03 << 3) // 陀螺仪量程±2000°/s

// MPU60X0_REG_ACCEL_CONFIG
#define MPU60X0_XACCEL_SELF_TEST (0x01 << 7) // 加速度计自检使能位
#define MPU60X0_YACCEL_SELF_TEST (0x01 << 6) // 加速度计自检使能位
#define MPU60X0_ZACCEL_SELF_TEST (0x01 << 5) // 加速度计自检使能位

#define MPU60X0_ACCEL_CONFIG_FS_MASK   (0x03 << 3) // 加速度计量程配置掩码
#define MPU60X0_ACCEL_CONFIG_FS_2G     (0x00 << 3) // 加速度计量程±2g
#define MPU60X0_ACCEL_CONFIG_FS_4G     (0x01 << 3) // 加速度计量程±4g
#define MPU60X0_ACCEL_CONFIG_FS_8G     (0x02 << 3) // 加速度计量程±8g
#define MPU60X0_ACCEL_CONFIG_FS_16G    (0x03 << 3) // 加速度计量程±16g

#define MPU60X0_ACCEL_CONFIG_DHPF_MASK     (0x07 << 0) // 加速度计数字高通滤波器配置掩码
#define MPU60X0_ACCEL_CONFIG_DHPF_DISABLED (0x00 << 0) // 禁用高通滤波器
#define MPU60X0_ACCEL_CONFIG_DHPF_5HZ      (0x01 << 0) // 高通滤波器截止频率5Hz
#define MPU60X0_ACCEL_CONFIG_DHPF_2P5HZ    (0x02 << 0) // 高通滤波器截止频率2.5Hz
#define MPU60X0_ACCEL_CONFIG_DHPF_1P25HZ   (0x03 << 0) // 高通滤波器截止频率1.25Hz
#define MPU60X0_ACCEL_CONFIG_DHPF_0P63HZ   (0x04 << 0) // 高通滤波器截止频率0.63Hz
#define MPU60X0_ACCEL_CONFIG_HOLD          (0x07 << 0) // 高通滤波器保持模式, 记录当前数据为锁定值，后续输出值为与锁定值的差异

// MPU60X0_REG_FF_THR 1LSB = 1mg
#define MPU60X0_FF_THR_0P1G          100 // 自由落体阈值0.1g
#define MPU60X0_FF_THR_0P2G          200 // 自由落体阈值0.2g

// MPU60X0_REG_FF_DUR 1LSB = 1ms
#define  MPU60X0_FF_DUR_50MS           50  // 自由落体持续时间50ms, 下落高度约为1.2cm
#define  MPU60X0_FF_DUR_100MS          100 // 自由落体持续时间100ms, 下落高度约为4.9cm
#define  MPU60X0_FF_DUR_200MS          200 // 自由落体持续时间200ms, 下落高度约为19.6cm

// MPU60X0_REG_MOT_THR 1LSB = 1mg
#define MPU60X0_MOT_THR_10MG          10 // 运动阈值10mg
#define MPU60X0_MOT_THR_20MG          20 // 运动阈值20mg
#define MPU60X0_MOT_THR_30MG          30 // 运动阈值30mg
#define MPU60X0_MOT_THR_40MG          40 // 运动阈值40mg
#define MPU60X0_MOT_THR_50MG          50 // 运动阈值50mg

// MPU60X0_REG_MOT_DUR 1LSB = 1ms
#define MPU60X0_MOT_DUR_10MS          10 // 运动持续时间10ms
#define MPU60X0_MOT_DUR_20MS          20 // 运动持续时间20ms
#define MPU60X0_MOT_DUR_40MS          40 // 运动持续时间40ms
#define MPU60X0_MOT_DUR_50MS          50 // 运动持续时间50ms

// MPU60X0_REG_ZMOT_THR 1LSB = 1mg
#define MPU60X0_ZMOT_THR_10MG         10 // 静止阈值10mg
#define MPU60X0_ZMOT_THR_20MG         20 // 静止阈值20mg
#define MPU60X0_ZMOT_THR_30MG         30 // 静止阈值30mg
#define MPU60X0_ZMOT_THR_40MG         40 // 静止阈值40mg
#define MPU60X0_ZMOT_THR_50MG         50 // 静止阈值50mg

// MPU60X0_REG_ZMOT_DUR 1LSB = 1ms
#define MPU60X0_ZMOT_DUR_10MS         10 // 静止持续时间10ms
#define MPU60X0_ZMOT_DUR_20MS         20 // 静止持续时间20ms
#define MPU60X0_ZMOT_DUR_30MS         30 // 静止持续时间30ms
#define MPU60X0_ZMOT_DUR_40MS         40 // 静止持续时间40ms
#define MPU60X0_ZMOT_DUR_50MS         50 // 静止持续时间50ms

// MPU60X0_REG_FIFO_EN
#define MPU60X0_TEMP_FIFO_EN  (0x01 << 7) // 温度FIFO使能位
#define MPU60X0_XG_FIFO_EN    (0x01 << 6) // X轴陀螺仪FIFO使能位
#define MPU60X0_YG_FIFO_EN    (0x01 << 5) // Y轴陀螺仪FIFO使能位
#define MPU60X0_ZG_FIFO_EN    (0x01 << 4) // Z轴陀螺仪FIFO使能位
#define MPU60X0_ACCEL_FIFO_EN (0x01 << 3) // 加速度计FIFO使能位(同时使能三轴)
#define MPU60X0_SLV2_FIFO_EN  (0x01 << 2) // I2C从设备2使能位
#define MPU60X0_SLV1_FIFO_EN  (0x01 << 1) // I2C从设备1使能位
#define MPU60X0_SLV0_FIFO_EN  (0x01 << 0) // I2C从设备0使能位

// MPU60X0_REG_I2C_MST_CTRL
#define MPU60X0_MULT_MST_EN   (0x01 << 7) // I2C主设备多主模式使能位
#define MPU60X0_WAIT_FOR_ES   (0x01 << 6) // 读取时等待外部传感器数据有效
#define MPU60X0_SLV_3_FIFO_EN (0x01 << 5) // 从设备3数据直接使写入FIFO
#define MPU60X0_I2C_MST_P_NSR (0x01 << 4) // 1：序列结束时发起STOP(建议，连续读取), 0：每次传输后发起STOP

#define MPU60X0_I2C_MST_CLK_MASK   (0x0F << 0) // I2C主设备时钟配置掩码
#define MPU60X0_I2C_MST_CLK_348KHZ (0x00 << 0) // I2C主设备时钟348kHz
#define MPU60X0_I2C_MST_CLK_333KHZ (0x01 << 0) // I2C主设备时钟333kHz
#define MPU60X0_I2C_MST_CLK_320KHZ (0x02 << 0) // I2C主设备时钟320kHz
#define MPU60X0_I2C_MST_CLK_308KHZ (0x03 << 0) // I2C主设备时钟308kHz
#define MPU60X0_I2C_MST_CLK_296KHZ (0x04 << 0) // I2C主设备时钟296kHz
#define MPU60X0_I2C_MST_CLK_286KHZ (0x05 << 0) // I2C主设备时钟286kHz
#define MPU60X0_I2C_MST_CLK_276KHZ (0x06 << 0) // I2C主设备时钟276kHz
#define MPU60X0_I2C_MST_CLK_267KHZ (0x07 << 0) // I2C主设备时钟267kHz
#define MPU60X0_I2C_MST_CLK_258KHZ (0x08 << 0) // I2C主设备时钟258kHz
#define MPU60X0_I2C_MST_CLK_500KHZ (0x09 << 0) // I2C主设备时钟500kHz
#define MPU60X0_I2C_MST_CLK_471KHZ (0x0A << 0) // I2C主设备时钟471kHz
#define MPU60X0_I2C_MST_CLK_444KHZ (0x0B << 0) // I2C主设备时钟444kHz
#define MPU60X0_I2C_MST_CLK_421KHZ (0x0C << 0) // I2C主设备时钟421kHz
#define MPU60X0_I2C_MST_CLK_400KHZ (0x0D << 0) // I2C主设备时钟400kHz(一般建议设该值)
#define MPU60X0_I2C_MST_CLK_381KHZ (0x0E << 0) // I2C主设备时钟381kHz
#define MPU60X0_I2C_MST_CLK_364KHZ (0x0F << 0) // I2C主设备时钟364kHz

// 从设备配置 - 通用
#define MPU60X0_I2C_SLV_RW_BIT        (0x01 << 7) // 从设备读写位, 0写1读
#define MPU60X0_I2C_SLV_EN_BIT        (0x01 << 7) // 从设备使能位， 开始传输数据，注意启用期间，主控制器MCU不能访问MPU60X0
// 从设备配置 - 从设备0到3
#define MPU60X0_I2C_SLV_BYTE_SW_BIT   (0x01 << 6) // 从设备字节交换位，0：不交换字节顺序，1：交换字节顺序，高低位互换
#define MPU60X0_I2C_SLV_REG_DIS_BIT   (0x01 << 5) // 从设备寄存器地址增量禁用， 1：禁止寄存器地址增量，0：允许寄存器地址增量
#define MPU60X0_I2C_SLV_GRP_BIT       (0x01 << 4) // 从设备组传输位， 1：将 SLV0 数据与 SLV1 数据合并存储，0：不合并
#define MPU60X0_I2C_SLV_LEN_MASK      (0x0F << 0) // 从设备传输长度掩码
// 从设备配置 - 仅从设备4
#define MPU60X0_I2C_SLV4_DO_MASK      (0xFF << 0) // 从设备4数据输出掩码
#define MPU60X0_I2C_SLV4_DI_MASK      (0xFF << 0) // 从设备4数据输入掩码
#define MPU60X0_I2C_SLV4_INT_EN_BIT   (0x01 << 7) // 从设备4中断使能位，1：使能中断，0：禁用中断
#define MPU60X0_I2C_SLV4_REG_DIS_BIT  (0x01 << 5) // 从设备4寄存器读取配置，1：读写寄存器数据，0：相当于读写无寄存器地址设备
#define MPU60X0_I2C_SLV4_MST_DLY_MASK (0x0F << 0) // 从设备4主设备延迟掩码，配置从设备4的I2C主设备延迟

// MPU60X0_REG_I2C_MST_STATUS
#define MPU60X0_I2C_MST_PASS_THROUGH  (0x01 << 7) // I2C主设备直通，即(AUX_DA/AUS_CLK)引脚直接连接到I2C总线, 1启用，0禁用
#define MPU60X0_I2C_MST_I2C_SLV4_DONE (0x01 << 6) // I2C从设备4传输完成位，1：传输完成，0：未完成
#define MPU60X0_I2C_MST_I2C_LOST_ARB  (0x01 << 5) // I2C主设备丢失仲裁位，1：丢失仲裁，0：未丢失
#define MPU60X0_I2C_MST_I2C_SLV4_NACK (0x01 << 4) // I2C从设备4未应答位，1：未应答，0：应答
#define MPU60X0_I2C_MST_I2C_SLV3_NACK (0x01 << 3) // I2C从设备3未应答位，1：未应答，0：应答
#define MPU60X0_I2C_MST_I2C_SLV2_NACK (0x01 << 2) // I2C从设备2未应答位，1：未应答，0：应答
#define MPU60X0_I2C_MST_I2C_SLV1_NACK (0x01 << 1) // I2C从设备1未应答位，1：未应答，0：应答
#define MPU60X0_I2C_MST_I2C_SLV0_NACK (0x01 << 0) // I2C从设备0未应答位，1：未应答，0：应答

// MPU60X0_REG_INT_PIN_CFG
#define MPU60X0_INT_LEVEL_BIT         (0x01 << 7) // 中断引脚电平触发位，1：高电平触发，0：低电平触发
#define MPU60X0_INT_OPEN_BIT          (0x01 << 6) // 中断引脚开漏输出位，1：开漏输出，0：推挽输出
#define MPU60X0_LATCH_INT_EN_BIT      (0x01 << 5) // 中断锁存使能，1：锁存(读取INT_STATUS清除)，0：非锁存(中断清除后立即清除)
#define MPU60X0_INT_RD_CLEAR_BIT      (0x01 << 4) // 中断读取清除位，1：读取INT_STATUS后清除中断(锁存模式需要置位)，0：不清除
#define MPU60X0_FSYNC_INT_LEVEL       (0x01 << 3) // FSYNC引脚中断电平触发位，1：高电平触发，0：低电平触发
#define MPU60X0_FSYNC_INT_EN_BIT      (0x01 << 2) // FSYNC引脚中断使能位，1：使能FSYNC中断，0：禁用FSYNC中断
#define MPU60X0_I2C_BYPASS_EN_BIT     (0x01 << 1) // I2C旁路使能位，1：使能I2C旁路模式(允许直接访问I2C总线)，0：禁用
#define MPU60X0_I2C_IF_DIS_BIT        (0x01 << 0) // I2C接口禁用位，1：禁用I2C接口，0：启用I2C接口

// MPU60X0_REG_INT_ENABLE, Bits 2 and 1 are reserved
#define MPU60X0_FF_INT_EN             (0x01 << 7) // 自由落体(free fall)中断使能位，1：使能自由落体中断，0：禁用
#define MPU60X0_MOT_INT_EN            (0x01 << 6) // 运动检测中断使能位，1：使能运动检测中断，0：禁用
#define MPU60X0_ZMOT_INT_EN           (0x01 << 5) // 静止检测中断使能位，1：使能静止检测中断，0：禁用
#define MPU60X0_FIFO_OFLOW_INT_EN     (0x01 << 4) // FIFO溢出中断使能位，1：使能FIFO溢出中断，0：禁用
#define MPU60X0_I2C_MST_INT_EN        (0x01 << 3) // I2C主设备中断使能位，1：使能I2C主设备中断，0：禁用
#define MPU60X0_DATA_RDY_INT_EN       (0x01 << 0) // 数据就绪中断使能位，1：使能数据就绪中断，0：禁用

// MPU60X0_REG_INT_STATUS, Bits 2 and 1 are reserved
#define MPU60X0_FF_INT_BIT            (0x01 << 7) // 自由落体中断状态位，1：发生自由落体事件，0：未发生
#define MPU60X0_MOT_INT_BIT           (0x01 << 6) // 运动检测中断状态位，1：发生运动事件，0：未发生
#define MPU60X0_ZMOT_INT_BIT          (0x01 << 5) // 静止检测中断状态位，1：发生静止事件，0：未发生
#define MPU60X0_FIFO_OFLOW_INT_BIT    (0x01 << 4) // FIFO溢出中断状态位，1：发生FIFO溢出事件，0：未发生
#define MPU60X0_I2C_MST_INT_BIT       (0x01 << 3) // I2C主设备中断状态位，1：发生I2C主设备事件，0：未发生
#define MPU60X0_DATA_RDY_INT_BIT      (0x01 << 0) // 数据就绪中断状态位，1：数据就绪，0：数据未就绪      

// MPU60X0_REG_MOT_DETECT_STATUS
#define MPU60X0_MOT_DETECT_XNEG_BIT   (0x01 << 7) // 运动检测X轴负向事件位，1：发生负向运动事件，0：未发生
#define MPU60X0_MOT_DETECT_XPOS_BIT   (0x01 << 6) // 运动检测X轴正向事件位，1：发生正向运动事件，0：未发生
#define MPU60X0_MOT_DETECT_YNEG_BIT   (0x01 << 5) // 运动检测Y轴负向事件位，1：发生负向运动事件，0：未发生
#define MPU60X0_MOT_DETECT_YPOS_BIT   (0x01 << 4) // 运动检测Y轴正向事件位，1：发生正向运动事件，0：未发生
#define MPU60X0_MOT_DETECT_ZNEG_BIT   (0x01 << 3) // 运动检测Z轴负向事件位，1：发生负向运动事件，0：未发生
#define MPU60X0_MOT_DETECT_ZPOS_BIT   (0x01 << 2) // 运动检测Z轴正向事件位，1：发生正向运动事件，0：未发生
#define MPU60X0_MOT_DETECT_ZRMOT_BIT  (0x01 << 0) // 运动检测静止事件位(ZERO MOTION)，1：发生静止事件，0：未发生

// MPU60X0_REG_I2C_MST_DELAY_CTRL
#define MPU60X0_I2C_MST_DELAY_ES_SHADOW_BIT (0x01 << 7) // I2C主设备延迟使能位，1：使能延迟，0：禁用延迟
#define MPU60X0_I2C_SLV4_DLY_EN_BIT         (0x01 << 4) // I2C从设备4延迟使能位，1：使能延迟，0：禁用延迟
#define MPU60X0_I2C_SLV3_DLY_EN_BIT         (0x01 << 3) // I2C从设备3延迟使能位，1：使能延迟，0：禁用延迟
#define MPU60X0_I2C_SLV2_DLY_EN_BIT         (0x01 << 2) // I2C从设备2延迟使能位，1：使能延迟，0：禁用延迟
#define MPU60X0_I2C_SLV1_DLY_EN_BIT         (0x01 << 1) // I2C从设备1延迟使能位，1：使能延迟，0：禁用延迟
#define MPU60X0_I2C_SLV0_DLY_EN_BIT         (0x01 << 0) // I2C从设备0延迟使能位，1：使能延迟，0：禁用延迟

// MPU60X0_REG_SIGNAL_PATH_RESET
#define MPU60X0_GYRO_RESET_BIT        (0x01 << 2) // 陀螺仪信号路径复位位，1：复位陀螺仪信号路径，0：不复位
#define MPU60X0_ACCEL_RESET_BIT       (0x01 << 1) // 加速度计信号路径复位位，1：复位加速度计信号路径，0：不复位
#define MPU60X0_TEMP_RESET_BIT        (0x01 << 0) // 温度传感器信号路径复位位，1：复位温度传感器信号路径，0：不复位


// MPU60X0_REG_MOT_DETECT_CTRL
#define MPU60X0_DETECT_ACCEL_ON_DELAY_MASK (0x03 << 4) // 运动检测加速度计开启延迟掩码
#define MPU60X0_DETECT_ACCEL_ON_DELAY_0MS  (0x00 << 4) // 运动检测加速度计开启延迟0ms
#define MPU60X0_DETECT_ACCEL_ON_DELAY_1MS  (0x01 << 4) // 运动检测加速度计开启延迟1ms
#define MPU60X0_DETECT_ACCEL_ON_DELAY_2MS  (0x02 << 4) // 运动检测加速度计开启延迟2ms
#define MPU60X0_DETECT_ACCEL_ON_DELAY_3MS  (0x03 << 4) // 运动检测加速度计开启延迟3ms

#define MPU60X0_DETECT_FF_COUNT_MASK  (0x03 << 2) // 自由落体检测计数掩码
#define MPU60X0_DETECT_FF_COUNT_1     (0x00 << 2) // 自由落体检测计数1次
#define MPU60X0_DETECT_FF_COUNT_2     (0x01 << 2) // 自由落体检测计数2次
#define MPU60X0_DETECT_FF_COUNT_4     (0x02 << 2) // 自由落体检测计数4次
#define MPU60X0_DETECT_FF_COUNT_8     (0x03 << 2) // 自由落体检测计数8次

#define MPU60X0_DETECT_MOT_COUNT_MASK (0x03 << 0) // 运动检测计数掩码
#define MPU60X0_DETECT_MOT_COUNT_1    (0x00 << 0) // 运动检测计数1次
#define MPU60X0_DETECT_MOT_COUNT_2    (0x01 << 0) // 运动检测计数2次
#define MPU60X0_DETECT_MOT_COUNT_4    (0x02 << 0) // 运动检测计数4次
#define MPU60X0_DETECT_MOT_COUNT_8    (0x03 << 0) // 运动检测计数8次

// MPU60X0_REG_USER_CTRL
#define MPU60X0_FIFO_EN          (0x01 << 6) // FIFO使能位，1：启用FIFO，0：禁用FIFO
#define MPU60X0_I2C_MST_EN       (0x01 << 5) // 1: AUX I2C由MPU60x0控制，0：AUX I2C由主控制器MCU控制
#define MPU60X0_I2C_IF_DIS       (0x01 << 4) // I2C接口禁用位，1：禁用主I2C接口,启用SPI接口，0：启用I2C接口，禁用SPI
#define MPU60X0_FIFO_RESET       (0x01 << 2) // FIFO复位位，1：复位FIFO，0：不复位
#define MPU60X0_I2C_MST_RESET    (0x01 << 1) // I2C主设备复位位，1：复位I2C主设备，0：不复位
#define MPU60X0_SIG_COND_RESET   (0x01 << 0) // 同时复位acc/gyro/temp，1：复位，0：不复位, 单独复位用SIGNAL_PATH_RESET

// MPU60X0_REG_PWR_MGMT_1
#define MPU60X0_DEVICE_RESET     (0x01 << 7) // 设备复位位，1：复位设备，0：不复位
#define MPU60X0_SLEEP            (0x01 << 6) // 睡眠模式位，1：进入睡眠模式，0：退出睡眠模式
#define MPU60X0_CYCLE            (0x01 << 5) // 循环模式位，1：启用循环模式，0：禁用循环模式
#define MPU60X0_TEMP_DIS         (0x01 << 3) // 温度传感器禁用位，1：禁用温度传感器，0：启用温度传感器

#define MPU60X0_CLKSEL_MASK          (0x07 << 0) // 时钟源选择掩码
#define MPU60X0_CLKSEL_INTERNAL      (0x00 << 0) // 内部8MHz振荡器
#define MPU60X0_CLKSEL_PLL_XGYRO     (0x01 << 0) // PLL X轴陀螺仪作为时钟源（推荐） // 1kHz 或 8kHz
#define MPU60X0_CLKSEL_PLL_YGYRO     (0x02 << 0) // PLL Y轴陀螺仪作为时钟源
#define MPU60X0_CLKSEL_PLL_ZGYRO     (0x03 << 0) // PLL Z轴陀螺仪作为时钟源
#define MPU60X0_CLKSEL_PLL_EXT32K    (0x04 << 0) // PLL 外部32.768kHz时钟作为时钟源
#define MPU60X0_CLKSEL_PLL_EXT19M    (0x05 << 0) // PLL 外部19.2MHz时钟作为时钟源
#define MPU60X0_CLKSEL_STOP          (0x07 << 0) // 停止时钟

// MPU60X0_REG_PWR_MGMT_2
#define MPU60X0_LP_WAKE_CTRL_MASK  (0x03 << 6) // LP_WAKE_CTRL位掩码
#define MPU60X0_LP_WAKE_CTRL_1P25HZ (0x00 << 6) // 低功耗唤醒频率1.25Hz
#define MPU60X0_LP_WAKE_CTRL_5HZ    (0x01 << 6) // 低功耗唤醒频率5Hz
#define MPU60X0_LP_WAKE_CTRL_20HZ   (0x02 << 6) // 低功耗唤醒频率20Hz
#define MPU60X0_LP_WAKE_CTRL_40HZ   (0x03 << 6) // 低功耗唤醒频率40Hz

#define MPU60X0_STBY_AXIS_MASK     (0x01 << 5) // 加速度和陀螺仪计待机位掩码
#define MPU60X0_STBY_XA        (0x01 << 5) // X轴加速度计待机位，1：待机，0：工作
#define MPU60X0_STBY_YA        (0x01 << 4) // Y轴加速度计待机位，1：待机，0：工作
#define MPU60X0_STBY_ZA        (0x01 << 3) // Z轴加速度计待机位，1：待机，0：工作
#define MPU60X0_STBY_XG        (0x01 << 2) // X轴陀螺仪待机位，1：待机，0：工作
#define MPU60X0_STBY_YG        (0x01 << 1) // Y轴陀螺仪待机位，1：待机，0：工作
#define MPU60X0_STBY_ZG        (0x01 << 0) // Z轴陀螺仪待机位，1：待机，0：工作
#define MPU60X0_STBY_ALL_AXIS  (MPU60X0_STBY_XA | MPU60X0_STBY_YA | MPU60X0_STBY_ZA | \
                                MPU60X0_STBY_XG | MPU60X0_STBY_YG | MPU60X0_STBY_ZG)
#define MPU60X0_WAKE_ALL_AXIS  (~MPU60X0_STBY_ALL_AXIS)

// MPU60X0_REG_WHO_AM_I
#define MPU60X0_WHO_AM_I_DEFAULT 0x68 // MPU60X0默认的WHO_AM_I寄存器值

// 注意：mpu60x0I2cReadReg8和mpu60x0I2cWriteReg8没有返回值, 而mpu60x0I2cBlockReadReg8和mpu60x0I2cBlockWriteReg8有返回值
// 根据情况可以将mpu60x0I2cReadReg8和mpu60x0I2cWriteReg8封装为函数，以便获取返回值，以及避免重复定义变量
#define mpu60x0I2cReadReg8(bus, reg, regData) do { \
        i2cReadReg8(bus, MPU6050_I2C_ADDRESS, reg, regData, 1); \
    } while (0)
#define mpu60x0I2cWriteReg8(bus, reg, regData) do { \
        uint8_t mpu60x0I2cWriteReg8data = regData; \
        i2cWriteReg8(bus, MPU6050_I2C_ADDRESS, reg, &mpu60x0I2cWriteReg8data, 1); \
    } while (0) // 1、mpu60x0I2cWriteReg8data名字取长点避免重名，2、do while(0)中间语句不能加注释
#define mpu60x0I2cBlockReadReg8(bus, reg, data, len) i2cReadReg8(bus, MPU6050_I2C_ADDRESS, reg, data, len)
#define mpu60x0I2cBlockWriteReg8(bus, reg, data, len) i2cWriteReg8(bus, MPU6050_I2C_ADDRESS, reg, data, len)

int mpu60x0Reset(Driver_t *driver);
int mpu60x0SetStandbyMode(Driver_t *driver, uint8_t stby_axis);
int mpu60x0SetSleepMode(Driver_t *driver, bool sleep);
int mpu60x0GetAccResolution(Driver_t *driver, float *resolution);
int mpu60x0GetGyroResolution(Driver_t *driver, float *resolution);
int mpu60x0GetAccSensitivity(Driver_t *driver, int *sensitivity);
int mpu60x0GetGyroSensitivity(Driver_t *driver, float *sensitivity);
int mpu60x0ReadAcc(Driver_t *driver, int16_t *acc_data, uint8_t len);
int mpu60x0ReadGyro(Driver_t *driver, int16_t *gyro_data, uint8_t len);
int mpu60x0ReadTemperature(Driver_t *driver, int16_t *temperature);
int mpu60x0Init(Driver_t *driver);

#endif // _MPU60X0_H