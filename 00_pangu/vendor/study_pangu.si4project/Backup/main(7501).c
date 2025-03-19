#include "stm32f1xx_hal.h"

#include<stdio.h>

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    int a = 0;
    printf("%d\n", a);
    return 0;
}
