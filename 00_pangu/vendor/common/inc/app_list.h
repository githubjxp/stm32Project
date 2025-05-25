/**
 *@file:
 * @author: jiangxiaoping
 * @data: 2025-04-24
 * @version: v1.0.0
 */

#ifndef _APP_LIST_H
#define _APP_LIST_H

#include "app.h"

#include <stdint.h>

typedef struct AppListNode_t {
	App_t* app;
	struct AppListNode_t* prev;
	struct AppListNode_t* next;
};
typedef struct AppListNode_t AppListNode_t;

void appInsertNodeAtHead(AppListNode_t** head, App_t* app);
void appInsertNodeAtTail(AppListNode_t** head, App_t* app);
void appDeleteNodeByTag(AppListNode_t** head, uint32_t tag);
AppListNode_t* appGetNodeByTag(AppListNode_t* head, uint32_t tag);
#endif	