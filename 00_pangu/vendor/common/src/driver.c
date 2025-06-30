/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-17
 * @version v1.0.0
 */
 
#include "driver.h"

#include "driver_list.h"
#include "print.h"
#include "tag.h"

static DriverListNode_t *g_driver_list_head = NULL;

Driver_t *getDriverByTag(const uint32_t tag)
{
    if (tag >= DRIVER_TAG_END) {
        PRINT_ERROR("tag is invalid!");
        return NULL;
    }

    DriverListNode_t *node = driverGetNodeByTag(g_driver_list_head, tag);
    if (node != NULL)
        return node->driver;

    PRINT_ERROR("driver tag%d not exist", tag);
    return NULL;
}

void driverRegister(Driver_t *driver)
{
    if (driver == NULL) {
        PRINT_ERROR("driver is NULL!");
        return;
    }

    if (driver->tag >= DRIVER_TAG_END) {
        PRINT_ERROR("driver tag is invalid!");
        return;
    }

    driverInsertNodeAtTail(&g_driver_list_head, driver);
}

extern uint32_t Image$$DRIVER_INIT_RAM$$Base;
uint32_t __attribute__((section(".driver_init_end"), used)) driver_init_end_symbol = 0;

static void runDriverRegister(void) {
    DriverInitFunc_t *driverInitStart = (DriverInitFunc_t *)&Image$$DRIVER_INIT_RAM$$Base;
    DriverInitFunc_t *driverInitEnd = (DriverInitFunc_t *)&driver_init_end_symbol;

    for (DriverInitFunc_t *func = driverInitStart; func < driverInitEnd; func++) {
        if (*func)
            (*func)(); // register driver
    }
}

void runDriverInit(void) {
    runDriverRegister();

    DriverListNode_t *current = g_driver_list_head;
    while (current != NULL) {
        if (current->driver->ops->init) {
            current->driver->ops->init(current->driver);
        }
        current = current->next;
    }
}
