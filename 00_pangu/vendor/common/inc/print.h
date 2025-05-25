/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-17
 * @version v1.0.0
 */

#ifndef _PRINT_H
#define _PRINT_H

// #include "portmacro.h"

#include "stm32f1xx.h"

#include <stdio.h>

#define PRINT_LEVEL_DEBUG  1
#define PRINT_LEVEL_INFO   2
#define PRINT_LEVEL_WARN   3
#define PRINT_LEVEL_ERROR  4
#define PRINT_LEVEL_NO_PRINT 5

#define PRINT_FOR_TEST_ENABLE 1

#ifndef PRINT_LEVEL
#define PRINT_LEVEL PRINT_LEVEL_INFO
#endif

// #define JXP_FOR_TEST
#ifndef JXP_FOR_TEST
#define LINE_BREAK "\r\n"
#define PRINT_DEBUG(fmt, ...) \
    do { \
        if (PRINT_LEVEL <= PRINT_LEVEL_DEBUG) { \
            printf("[%u][DEBUG] %s: "fmt""LINE_BREAK, HAL_GetTick(), __func__, ##__VA_ARGS__); \
        } \
    } while (0)

#define PRINT_INFO(fmt, ...) \
    do { \
        if (PRINT_LEVEL <= PRINT_LEVEL_INFO) { \
            printf("[%u][INFO] %s: "fmt""LINE_BREAK, HAL_GetTick(), __func__, ##__VA_ARGS__); \
        } \
    } while (0)

#define PRINT_WARN(fmt, ...) \
    do { \
        if (PRINT_LEVEL <= PRINT_LEVEL_WARN) { \
            printf("[%u][WARN] %s: "fmt""LINE_BREAK, HAL_GetTick(), __func__, ##__VA_ARGS__); \
        } \
    } while (0)

#define PRINT_ERROR(fmt, ...) \
    do { \
        if (PRINT_LEVEL <= PRINT_LEVEL_ERROR) { \
            printf("[%u][ERROR] %s: "fmt""LINE_BREAK, HAL_GetTick(), __func__, ##__VA_ARGS__); \
        } \
    } while (0)

#define PRINT_FOR_TEST(fmt, ...) \
    do { \
        if (PRINT_FOR_TEST_ENABLE) { \
            printf("[%u][TEST] %s: "fmt""LINE_BREAK, HAL_GetTick(), __func__, ##__VA_ARGS__); \
        } \
    } while (0)
#else
#define PRINT_INFO
#define PRINT_DEBUG
#define PRINT_WARN
#define PRINT_ERROR
#define PRINT_FOR_TEST
#endif

#endif
