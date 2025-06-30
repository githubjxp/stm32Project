/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-16
 * @version v1.0.0
 * @brief UART
 **/

#include "uart_config.h"

#include "NVIC_config.h"
#include "dma_config.h"
#include "print.h"
#include "stm32f1xx.h"

#include "stdbool.h"

uint8_t g_uart1RxBuff0[UART_RX_BUFF_SIZE] = {0};
uint8_t g_uart1RxBuff1[UART_RX_BUFF_SIZE] = {0};

/**
 * Note: If USART1, USART2, or USART3 are configured to use synchronous mode,
 * you must use the USART functions to perform operations. 
 * By default, all USART ports are configured to use asynchronous mode.
 */
static UART_HandleTypeDef huart[] = {
    {
        .Instance = USART1,
        .Init.BaudRate = 115200,
        .Init.WordLength = UART_WORDLENGTH_8B,
        .Init.StopBits = UART_STOPBITS_1,
        .Init.Parity = UART_PARITY_NONE,
        .Init.Mode = UART_MODE_TX_RX,
        .Init.HwFlowCtl = UART_HWCONTROL_NONE,
        .Init.OverSampling = UART_OVERSAMPLING_16,

        // 不需要配置这些参数，传输时传入，ptr会自动变化
        // .pRxBuffPtr = g_uart1RxBuff0,
        // .RxXferSize =  UART_RX_BUFF_SIZE,
        // .RxXferCount = UART_RX_BUFF_SIZE,
        // .pTxBuffPtr = g_uart1TxBuffer,
        // .TxXferSize = UART_TX_BUFFER_SIZE,
        // .TxXferCount = UART_TX_BUFFER_SIZE,
    },
};

UartConfig_t g_uartConfigTbl[] = {
    {
        .uart = &huart[0],
        .nvicConfig = {
            .irqn = USART1_IRQn,
            .preemptPriority = NVIC_PRIORITY_UART1, // configMAX_SYSCALL_INTERRUPT_PRIORITY
            .subPriority = 0,
        },
        .dmaTxFuncId = DMA_FUNC_ID_USART1_TX,
        .dmaRxFuncId = DMA_FUNC_ID_USART1_RX,
        .uartFuncId = UART_FUNC_ID_PC_COMMUNICATION,
        .status = HAL_ERROR,
        .rxBuff0 = {
            .buff = g_uart1RxBuff0,
            .size = UART_RX_BUFF_SIZE,
            .pos = 0,
        },
        .rxBuff1 = {
            .buff = g_uart1RxBuff1,
            .size = UART_RX_BUFF_SIZE,
            .pos = 0,
        },
        .rxBuffState = UART_BUFF_STATE_UNUSED,
    }
};

UartConfig_t *getUartConfigByFuncID(uint32_t funcId)
{
    for (int i = 0; i < sizeof(g_uartConfigTbl) / sizeof(g_uartConfigTbl[0]); i++) {
        if (g_uartConfigTbl[i].uartFuncId == funcId)
            return &g_uartConfigTbl[i];
    }

    return NULL;
}

UART_HandleTypeDef *getUartHandleByFuncID(uint32_t funcId)
{
    for (int i = 0; i < sizeof(g_uartConfigTbl) / sizeof(g_uartConfigTbl[0]); i++) {
        if (g_uartConfigTbl[i].uartFuncId == funcId)
            return g_uartConfigTbl[i].uart;
    }

    return NULL;
}

static void enableUartClk(USART_TypeDef *UARTx)
{
    if (UARTx == USART1)
        __HAL_RCC_USART1_CLK_ENABLE();
    else if (UARTx == USART2)
        __HAL_RCC_USART2_CLK_ENABLE();
    else if (UARTx == USART3)
        __HAL_RCC_USART3_CLK_ENABLE();
    else if (UARTx == UART4)
        __HAL_RCC_UART4_CLK_ENABLE();
    else if (UARTx == UART5)
        __HAL_RCC_UART5_CLK_ENABLE();
}

void uartInit(void)
{
    int i;
    int ret;

    for (i = 0; i < sizeof(g_uartConfigTbl) / sizeof(g_uartConfigTbl[0]); i++) {
        enableUartClk(g_uartConfigTbl[i].uart->Instance);
        ret = HAL_UART_Init(g_uartConfigTbl[i].uart);
        if (ret != HAL_OK) {
            g_uartConfigTbl[i].status = HAL_ERROR;
            continue;
        } else {
            // uart nvic init
            HAL_NVIC_SetPriority(g_uartConfigTbl[i].nvicConfig.irqn, g_uartConfigTbl[i].nvicConfig.preemptPriority,
                g_uartConfigTbl[i].nvicConfig.subPriority);
            HAL_NVIC_EnableIRQ(g_uartConfigTbl[i].nvicConfig.irqn);

            // uart dma init
            DMA_HandleTypeDef *dmaTx = getDmaHandleByFuncID(g_uartConfigTbl[i].dmaTxFuncId);
            DMA_HandleTypeDef *dmaRx = getDmaHandleByFuncID(g_uartConfigTbl[i].dmaRxFuncId);
            if (dmaTx != NULL) {
                g_uartConfigTbl[i].uart->hdmatx = dmaTx; // 待优化：移除该逻辑，预期不影响代码逻辑
                __HAL_LINKDMA(g_uartConfigTbl[i].uart, hdmatx, *dmaTx);
            }
            if (dmaRx != NULL) {
                g_uartConfigTbl[i].uart->hdmarx = dmaRx; // 待优化：移除该逻辑，预期不影响代码逻辑
                __HAL_LINKDMA(g_uartConfigTbl[i].uart, hdmarx, *dmaRx);
            }

            // uart it init
            HAL_UART_RegisterRxEventCallback(g_uartConfigTbl[i].uart, HAL_UARTEx_RxEventCallback); // just for uart idle
            HAL_UART_RegisterCallback(g_uartConfigTbl[i].uart, HAL_UART_TX_COMPLETE_CB_ID, HAL_UART_TxCpltCallback);
            HAL_UART_RegisterCallback(g_uartConfigTbl[i].uart, HAL_UART_RX_COMPLETE_CB_ID, HAL_UART_RxCpltCallback);
            HAL_UART_RegisterCallback(g_uartConfigTbl[i].uart, HAL_UART_ERROR_CB_ID, HAL_UART_ErrorCallback);

            g_uartConfigTbl[i].status = HAL_OK;

            if (g_uartConfigTbl[i].uart->hdmarx != NULL) {
                HAL_UARTEx_ReceiveToIdle_DMA(g_uartConfigTbl[i].uart, g_uart1RxBuff0, UART_RX_BUFF_SIZE);
            } else {
                HAL_UARTEx_ReceiveToIdle_IT(g_uartConfigTbl[i].uart, g_uart1RxBuff0, UART_RX_BUFF_SIZE);   
            }
            g_uartConfigTbl[i].rxBuffState = UART_BUFF_STATE_BUFF0_RECEIVING;
        }   
    }
}

// keil use fputc, but define gunc ??
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE* f)
#endif /* __GNUC__ */

// Redirect printf to UART
int fputc(int ch, FILE* f)
{
    HAL_UART_Transmit(&huart[0], (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
