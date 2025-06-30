#include "main.h"

#include "app.h"
#include "dma_config.h"
#include "dma_it.h"
#include "driver.h"
#include "gpio_config.h"
#include "i2c_config.h"
#include "spi_config.h"
#include "print.h"
#include "uart_config.h"
#include "uart_it.h"

#include "stm32f1xx.h"
#include "trcRecorder.h"

#include <stdio.h>

extern void SystemClock_Config(void);
extern void MCU_Base_Info_Print(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    // driver init
    gpioInit();
    dmaInit();
    uartInit();
    i2cInit();
    spiInit();

    // app init
    runDriverInit();
    runAppInit();

    // MCU info
    MCU_Base_Info_Print();

    // xTraceEnable(TRC_START); // ???????????
    // PRINT_INFO("Trace recorder started");

    // HAL_Delay(1000);
    taskEnter();
  
    while(1) {
        PRINT_INFO("main loop");
        HAL_Delay(2000);
    }
}

void MCU_Base_Info_Print(void)
{
    PRINT_INFO("mcu id: %x", HAL_GetDEVID());
    PRINT_INFO("mcu rev: %x", HAL_GetREVID());
    PRINT_INFO("mcu uid: %x %x %x", HAL_GetUIDw0(), HAL_GetUIDw1(), HAL_GetUIDw2());
    PRINT_INFO("mcu vtor: %x", SCB->VTOR);
    PRINT_INFO("mcu stack: %x", __get_MSP());
    PRINT_INFO("mcu heap: %x", __get_MSP() - 0x2000); // 8KB SRAM
    PRINT_INFO("system clock: %d", HAL_RCC_GetSysClockFreq());
    PRINT_INFO("system tick: %d", HAL_GetTickFreq());
    PRINT_INFO("SystemCoreClock: %d", SystemCoreClock);
    PRINT_INFO("uwTickPrio: %d", uwTickPrio);
}

static void Error_Handler(void)
{
  while (1)
  {
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
    HAL_Delay(500);
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

