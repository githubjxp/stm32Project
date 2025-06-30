/**
 *@file:
 * @author: jiangxiaoping
 * @data: 2025-04-24
 * @version: v1.0.0
 */

#ifndef _DRIVER_LIST_H
#define _DRIVER_LIST_H

#include "driver.h"

#include <stdint.h>

typedef struct DriverListNode_t {
	Driver_t* driver;
	struct DriverListNode_t* prev;
	struct DriverListNode_t* next;
} DriverListNode_t;

void driverInsertNodeAtHead(DriverListNode_t** head, Driver_t* driver);
void driverInsertNodeAtTail(DriverListNode_t** head, Driver_t* driver);
void driverDeleteNodeByTag(DriverListNode_t** head, uint32_t tag);
DriverListNode_t* driverGetNodeByTag(DriverListNode_t* head, uint32_t tag);
#endif	// _DRIVER_LIST_H