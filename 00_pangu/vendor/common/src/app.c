/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-17
 * @version v1.0.0
 */
 
#include "app.h"

#include "app_list.h"
#include "print.h"
#include "tag.h"

static AppListNode_t *g_app_list_head = NULL;
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

extern uint32_t Image$$APP_INIT_RAM$$Base;
uint32_t __attribute__((section(".app_init_end"), used)) app_init_end_symbol = 0;

static void runAppRegister(void) {
    AppInitFunc_t *appInitStart = (AppInitFunc_t *)&Image$$APP_INIT_RAM$$Base;
    AppInitFunc_t *appInitEnd = (AppInitFunc_t *)&app_init_end_symbol;

    for (AppInitFunc_t *func = appInitStart; func < appInitEnd; func++) {
        if (*func)
            (*func)(); // register app
    }
}

void runAppInit(void) {
    runAppRegister();

    AppListNode_t *current = g_app_list_head;
    while (current != NULL) {
        if (current->app->ops->init) {
            current->app->ops->init(current->app);
        }
        current = current->next;
    }
}

