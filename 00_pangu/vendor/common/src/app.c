/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-17
 * @version v1.0.0
 */
 
#include "app.h"

#include "print.h"
#include "tag.h"

#include <stdlib.h>

AppListNode_t *appCreateNode(App_t *app)
{
    if (app == NULL) {
        PRINT_ERROR("app is NULL!");
        return NULL; 
    }

    AppListNode_t *newNode = (AppListNode_t *)malloc(sizeof(AppListNode_t));
    if (newNode == NULL) {
        PRINT_ERROR("AppListNode_t malloc failed!");
        return NULL;
    }

    newNode->app = app;
    newNode->prev = NULL;
    newNode->next = NULL;
}

void appInsertNodeAtHead(AppListNode_t **head, App_t *app)
{
    AppListNode_t *newNode = appCreateNode(app);
    if (newNode == NULL)
        return;

    if (*head == NULL) {
        *head = newNode;
    } else {
        (*head)->prev = newNode;
        newNode->next = *head;
        *head = newNode;
    }
}

void appInsertNodeAtTail(AppListNode_t **head, App_t *app)
{
    AppListNode_t *newNode = appCreateNode(app);
    if (newNode == NULL)
        return;

    if (*head == NULL) {
        *head = newNode;
    } else {
        AppListNode_t *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
        newNode->prev = current;
    }
}

void appDeleteNodeByTag(AppListNode_t **head, uint32_t tag)
{
    if (*head == NULL || tag >= APP_TAG_END) {
        PRINT_DEBUG("head is NULL or tag is invalid");
        return;
    }

    AppListNode_t *current = *head;
    while(current != NULL) {
        if (current->app->tag == tag) {
            if (current->prev == NULL) {
                current = current->next;
            } else {
                current->prev->next = current->next;
                current->next->prev = current->prev;
                free(current);
            }
            PRINT_INFO("app tag%d is free", tag);
        }
        current = current->next;
    }
}

AppListNode_t *appGetNodeByTag(AppListNode_t *head, uint32_t tag)
{
    AppListNode_t *current = head;
    while (current != NULL) {
        if (current->app->tag == tag) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

AppListNode_t *g_app_list_head = NULL;
void appRegister(App_t *app)
{
    if (app == NULL) {
        PRINT_ERROR("app is NULL!");
        return;
    }

    if (app->tag >= APP_TAG_END) {
        PRINT_ERROR("app tag is invalid!");
        return;
    }

    appInsertNodeAtTail(&g_app_list_head, app);
}

// extern uint32_t Image$$APP_INIT$$Base;
// extern uint32_t Image$$APP_INIT$$Limit;

// void getAppInitSectionValues(void) {
//     uint32_t appInitStart = Image$$APP_INIT$$Base;
//     uint32_t appInitEnd = Image$$APP_INIT$$Limit;
//     for (AppInitFunc_t *func = appInitStart; func < appInitEnd; func++) {
//         if (*func) {
//             (*func)(); // 执行初始化函数
//         }
//         break;
//     }
// }

// void runAppInit(void) {
//     getAppInitSectionValues();
// }

#include "led.h"
void runAppInit(void) {
    ledAppInit();
}
