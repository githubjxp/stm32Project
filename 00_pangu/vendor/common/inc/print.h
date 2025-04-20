/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-17
 * @version v1.0.0
 */

#ifndef _PRINT_H
#define _PRINT_H

// #include "portmacro.h"

#include <stdio.h>

#define portENTER_CRITICAL() 
#define portEXIT_CRITICAL() // Exit critical section

#define PRINT_LEVEL_DEBUG  1
#define PRINT_LEVEL_INFO   2
#define PRINT_LEVEL_WARN   3
#define PRINT_LEVEL_ERROR  4

#ifndef PRINT_LEVEL
#define PRINT_LEVEL PRINT_LEVEL_INFO
#endif

#define PRINT_DEBUG(fmt, ...) \
    do { \
        if (PRINT_LEVEL <= PRINT_LEVEL_DEBUG) { \
            portENTER_CRITICAL(); \
            printf("[DEBUG] %s:" fmt "\n", __func__, ##__VA_ARGS__); \
            portEXIT_CRITICAL(); \
        } \
    } while (0)

#define PRINT_INFO(fmt, ...) \
    do { \
        if (PRINT_LEVEL <= PRINT_LEVEL_INFO) { \
            portENTER_CRITICAL(); \
            printf("[INFO] %s:" fmt "\n", __func__, ##__VA_ARGS__); \
            portEXIT_CRITICAL(); \
        } \
    } while (0)

#define PRINT_WARN(fmt, ...) \
    do { \
        if (PRINT_LEVEL <= PRINT_LEVEL_WARN) { \
            portENTER_CRITICAL(); \
            printf("[WARN] %s:" fmt "\n", __func__, ##__VA_ARGS__); \
            portEXIT_CRITICAL(); \
        } \
    } while (0)

#define PRINT_ERROR(fmt, ...) \
    do { \
        if (PRINT_LEVEL <= PRINT_LEVEL_ERROR) { \
            portENTER_CRITICAL(); \
            printf("[ERROR] %s:" fmt "\n", __func__, ##__VA_ARGS__); \
            portEXIT_CRITICAL(); \
        } \
    } while (0)

#endif
