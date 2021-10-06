#include <msp430.h>
#include <stdio.h>
#include <stdint.h>

#include "platform/platform.h"
#include "platform/gpio/gpio.h"
#include "platform/timer/timerA.h"

#include "platform/kernel/adk.h"
#include "platform/uart1/uart1.h"

uint8_t task1_stack[128] __attribute__((aligned(16)));

uint8_t task2_stack[128] __attribute__((aligned(16)));

// void delay(uint32_t ms)
// {
//     uint32_t i, j;
//     for (i = 0; i < ms; i++)
//         for (j = 0; j < 34U; j++)
//         {
//             __nop();
//             __nop();
//         }
// }

static void schedule_and_run(void);
static void Task1(void);
static void Task2(void);
static void IdleTaskHook(void);

static inline void pwr_12v(uint8_t state)
{
    if (state == 1)
    {
        CNT12V_OUT |= (1 << CNT12V_PIN);
    }
    else
    {
        CNT12V_OUT &= ~(1 << CNT12V_PIN);
    }
}

static inline void pwr_lg(uint8_t state)
{
    if (state == 1)
    {
        CNTLG_OUT |= (1 << CNTLG_PIN);
    }
    else
    {
        CNTLG_OUT &= ~(1 << CNTLG_PIN);
    }
}

int main(void)
{
    /* Initialize platform */
    platform_init();

    /* Initialize GPIO */
    gpio_init();

    /* Enable coil logic power */
    pwr_12v(1);
    pwr_lg(1);

    /* UART init */
    uart1_init(UART_BAUDRATE_9600, UART_FORMAT_8N1);

    uart1_send_str((uint8_t *)"DEMO V1.0\r\n", 0);

    schedule_and_run();

    while (1)
    {
        ;
    }

    return 0;
}

static void schedule_and_run(void)
{
    adk_init();

    adk_task_add(Task1, task1_stack, sizeof(task1_stack));
    adk_task_add(Task2, task2_stack, sizeof(task2_stack));

    adk_start(IdleTaskHook);

    while (1)
        ;
}

static void Task1(void)
{
    uart1_send_str((uint8_t *)"[1]Task1 started\r\n", 0);

    while (1)
    {
        DRVA_OUT |= (1 << DRVA_PIN);
        adk_delay(ADK_MS_TO_TICKS(1000));

        DRVA_OUT &= ~(1 << DRVA_PIN);
        adk_delay(ADK_MS_TO_TICKS(1000));
    }
}

static void Task2(void)
{
    uart1_send_str((uint8_t *)"[2]Task2 started\r\n", 0);

    while (1)
    {
        DRVB_OUT |= (1 << DRVB_PIN);
        adk_delay(ADK_MS_TO_TICKS(100));

        DRVB_OUT &= ~(1 << DRVB_PIN);
        adk_delay(ADK_MS_TO_TICKS(100));
    }
}

static void IdleTaskHook(void)
{
    /* Simple put the CPU into lowpower mode. */
    _BIS_SR(LPM3_bits);
}