/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-17
 * @version v1.0.0
 */

#ifndef _APP_H
#define _APP_H

#include <stdint.h>

extern struct AppOps_t;
typedef struct AppOps_t AppOps_t;

typedef struct App_t {
   uint32_t tag;
   uint8_t *appData;
   uint32_t appDataLen;
   void *priv;
   AppOps_t *ops;
} App_t;

struct AppOps_t {
   int32_t (*init)(App_t *app);
   int32_t (*open)(App_t *app);
   int32_t (*close)(App_t *app);
   int32_t (*config)(App_t *app, void *config); 
   int32_t (*processData)(App_t *app, uint32_t tag, uint8_t *data, uint32_t len);
   int32_t (*setInterval)(App_t *app, uint32_t interval);
};

typedef void (*AppInitFunc_t)(void);
#define APP_INIT(func) __attribute__((section(".app_init"), used)) AppInitFunc_t func##_init = func;

App_t *getAppByTag(const uint32_t tag);
void appRegister(App_t *app);
void runAppInit(void);

#endif // _APP_H
