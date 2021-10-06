#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "platform/timer/timerA.h"
#include "platform/gpio/gpio.h"
#include "platform/uart1/uart1.h"
#include "adk.h"
#include "adk_config.h"

/* 
 * Structure to store information about the task
 */
struct adk_task_info
{
    /* 
     * First member points to the current stack pointer
     * saved before switchting context 
     */
    volatile uint16_t *taskSP;

    /*
     * Stores an ID for a task
     * Set to task number at the task creation
     */
    volatile uint16_t taskid;

    /*
     * Used for delay purposes
     * If this count is greater than 0, 
     * count is decremented in every tick ISR
     * Till count is greater than 0, 
     * task will not be switched in.
     */
    volatile uint16_t taskDelay;

    /* 
     * Base of the stack at the initialization.
     */
    volatile uint16_t *stackbase;

    /*
     * Pointer to task function
     */
    volatile TaskFunction_t run;
};

/*************************************************************************/
/* Variables to store current inforation about schedular                 */
/*************************************************************************/

/* Struct array to store task information of
 * number of tasks set using NUM_TASKS
 */
volatile struct adk_task_info adk_tasks[ADK_CONFIG_NUM_TASKS];

/* Index of current task info in adk_tasks array */
static volatile uint16_t adk_current_task_idx = -1;

/* Stores current number of tasks including "idle task" */
static volatile uint16_t adk_task_count = 0;

/* Points to current task info from adk_tasks array */
static volatile uint16_t *currentTCB;

/* Tick counter increamented every tick */
static volatile uint32_t adk_tick = 0;

/* Idle task stack */
uint8_t adk_idle_task_stack[32] __attribute__((aligned(16)));

/* Hook function pointer for idle task hook */
adk_idle_task_hook_t adk_idle_task_hook = NULL;

/* 
 * Idle task function
 * 
 * If adk_idle_task_hook set (if not NULL),
 * task will call idle_task_hook repeatedly
 * 
 * If not (if NULL), it will bw a free running while loop
 * 
 */
static void adk_idle_task(void)
{
    while (1)
    {
        if (adk_idle_task_hook)
            adk_idle_task_hook();
    }
}

/*
 * Increament the tick
 * 
 * Also decrement ticks for all the tasks whose tickDelay is
 * greater than 0.
 * 
 */
static void adk_increment_tick(void)
{
    int i;
    adk_tick++;
    for (i = 1; i < adk_task_count; i++)
    {
        if (adk_tasks[i].taskDelay > 0)
            adk_tasks[i].taskDelay--;
    }
}

/* 
 * This function will switch context
 */
static void adk_switch_context(void)
{
    PULSE_OUT_OUT |= (1 << PULSE_OUT_PIN);

    /* Get the next task with zero taskDelay */

    /* As idle task will always have zero taskDelay
     * It will get switched in.
     */

    // TODO: In first iteration, should skip idle task i.e. task idx 0
    do
    {
        adk_current_task_idx++;
        if (adk_current_task_idx == adk_task_count)
            adk_current_task_idx = 0;

        /* If taskDelay is zero, break out of the loop */
        if (adk_tasks[adk_current_task_idx].taskDelay == 0)
            break;

    } while (1);

    /* Set currentTCB to appropriate info struct in adk_tasks array */
    currentTCB = (uint16_t *)&adk_tasks[adk_current_task_idx];
    PULSE_OUT_OUT &= ~(1 << PULSE_OUT_PIN);
}

/*
 * Timer ISR to switch the context
 */
__attribute__((naked)) void adk_yield(void)
{
    __disable_interrupt();

    /* Simulate SR push as if inside an interrupt */
    asm volatile("push r2");
    /* Make sure that interrupts are enabled in SR */
    asm volatile("bis.w #128,0(r1)");

    asm volatile("push	r4						\n\t"
                 "push	r5						\n\t"
                 "push	r6						\n\t"
                 "push	r7						\n\t"
                 "push	r8						\n\t"
                 "push	r9						\n\t"
                 "push	r10						\n\t"
                 "push	r11						\n\t"
                 "push	r12						\n\t"
                 "push	r13						\n\t"
                 "push	r14						\n\t"
                 "push	r15						\n\t");

    __asm__ __volatile__(
        "mov.w %0,R12 \n\t \
         mov.w SP,@R12"
        : "=m"(currentTCB)
        :);

    adk_switch_context();

    __asm__ __volatile__(
        "mov.w %0,R12 \n\t \
         mov.w @R12,R1"
        : "=m"(currentTCB)
        :);

    asm volatile("pop	r15						\n\t"
                 "pop	r14						\n\t"
                 "pop	r13						\n\t"
                 "pop	r12						\n\t"
                 "pop	r11						\n\t"
                 "pop	r10						\n\t"
                 "pop	r9						\n\t"
                 "pop	r8						\n\t"
                 "pop	r7						\n\t"
                 "pop	r6						\n\t"
                 "pop	r5						\n\t"
                 "pop	r4						\n\t"
                 "bic	#(0xf0),0(r1)			\n\t"
                 "reti							\n\t");
}

/*
 * Timer ISR to switch the context
 */
__attribute__((interrupt(TIMERA0_VECTOR), naked)) void Timer_A(void)
{
    /* Push all scratchpad registers to stack */
    /* R4-R15 are scratchpad registers */
    asm volatile("push	r4						\n\t"
                 "push	r5						\n\t"
                 "push	r6						\n\t"
                 "push	r7						\n\t"
                 "push	r8						\n\t"
                 "push	r9						\n\t"
                 "push	r10						\n\t"
                 "push	r11						\n\t"
                 "push	r12						\n\t"
                 "push	r13						\n\t"
                 "push	r14						\n\t"
                 "push	r15						\n\t");

    /* Save current SP(stack pointer) in the first member of
     * task info struct
     */
    __asm__ __volatile__(
        "mov.w %0,R12 \n\t \
         mov.w SP,@R12"
        : "=m"(currentTCB)
        :);

    PULSE_OUT_OUT |= (1 << PULSE_OUT_PIN);
    adk_increment_tick();
    adk_switch_context();
    PULSE_OUT_OUT &= ~(1 << PULSE_OUT_PIN);

    /* Restore stack pointer of switched task */
    __asm__ __volatile__(
        "mov.w %0,R12 \n\t \
         mov.w @R12,R1"
        : "=m"(currentTCB)
        :);

    /* Pop scratchpad registers in order they were pushed onto stack
     * Then make sure that no LPM bits are set in SR
     * After that issue RETI which will pop SR from stack and set PC
     * to appropriate return address
     */
    asm volatile("pop	r15						\n\t"
                 "pop	r14						\n\t"
                 "pop	r13						\n\t"
                 "pop	r12						\n\t"
                 "pop	r11						\n\t"
                 "pop	r10						\n\t"
                 "pop	r9						\n\t"
                 "pop	r8						\n\t"
                 "pop	r7						\n\t"
                 "pop	r6						\n\t"
                 "pop	r5						\n\t"
                 "pop	r4						\n\t"
                 "bic	#(0xf0),0(r1)			\n\t"
                 "reti							\n\t");
}

/* 
 * This function will init the schedular 
 */
void adk_init(void)
{
    int i = 0;

    __disable_interrupt();

    for (i = 0; i < ADK_CONFIG_NUM_TASKS; i++)
    {
        memset((void *)&adk_tasks[i], 0, sizeof(struct adk_task_info));
    }

    timerA_init(ADK_CONFIG_TICKS_PER_SECOND);

    /* Set task count to 0 */
    adk_task_count = 0;

    /* Set current task index to -1 and currentTCB to NULL,
     * reflecting no tasks is currently active and 
     * schedular is deactive */
    adk_current_task_idx = -1;
    currentTCB = NULL;

    /* Add idle task */
    adk_task_add(adk_idle_task, adk_idle_task_stack, sizeof(adk_idle_task_stack));
}

/* 
 * Delay function which will set taskDelay to set number of ticks
 * 
 * This will prevent schedular from scheduling this task for execution for
 * given number of ticks. This will immediately switch to next task
 * 
 * @param ticks:    Number of ticks to block current task
 * 
 */
void adk_delay(uint32_t ticks)
{
    __disable_interrupt();
    adk_tasks[adk_current_task_idx].taskDelay = ticks;
    __enable_interrupt();

    adk_yield();
}

/* 
 * This function will initialize the stack frame for new task
 * 
 * @param sTop: current top of stack
 * 
 * @param run: Function pointer to the task function
 * 
 * @return: Returns the new stack top
 * 
 */
static inline uint16_t *adk_stackframe_init(uint16_t *sTop, TaskFunction_t run)
{
    /* Push start address of task funtion*/
    *sTop = (uint16_t)run;
    sTop--;

    /* Set SR Make sure interrupts are enabled upon first context switch*/
    *sTop = (uint16_t)0x0008;
    sTop--;

    /* Make R15-R4 as 0 */
    *sTop = (uint16_t)0xFFFF;
    sTop--;
    *sTop = (uint16_t)0xEEEE;
    sTop--;
    *sTop = (uint16_t)0xDDDD;
    sTop--;
    *sTop = (uint16_t)0xCCCC;
    sTop--;
    *sTop = (uint16_t)0xBBBB;
    sTop--;
    *sTop = (uint16_t)0xAAAA;
    sTop--;
    *sTop = (uint16_t)0x9999;
    sTop--;
    *sTop = (uint16_t)0x8888;
    sTop--;
    *sTop = (uint16_t)0x7777;
    sTop--;
    *sTop = (uint16_t)0x6666;
    sTop--;
    *sTop = (uint16_t)0x5555;
    sTop--;
    *sTop = (uint16_t)0x4444;

    return sTop;
}

/*
 * This function adds task to schedular
 * 
 * @param run: Function pointer to the task function
 * 
 * @param stack: Pointer to the stack buffer to be provided by user app
 * 
 * @param stack_size: Size of stack buffer provided by application
 * 
 * @return: Returns 0 if task is initialized successfully
 *          Else returns -1, if max number of tasks are already added
 * 
 */
int adk_task_add(TaskFunction_t run, uint8_t *stack, uint16_t stack_size)
{
#ifdef DEBUG
    char str[100];
#endif

    volatile struct adk_task_info *new_task;

    /* Check if tasks added reached max number of tasks */
    if (adk_task_count >= ADK_CONFIG_NUM_TASKS)
        return -1;

    /* Get the task info struct */
    new_task = &adk_tasks[adk_task_count++];

    /* Set all members of the struct like stakbase, taskid, 
     * run funtion pointer 
     */
    new_task->stackbase =
        (uint16_t *)(((uint16_t)&stack[stack_size - 1]) & ~0x0001);
    new_task->taskid = adk_task_count;
    new_task->run = run;
    new_task->taskDelay = 0;

    /* Init stack frame */
    new_task->taskSP = adk_stackframe_init((uint16_t *)new_task->stackbase, run);

#ifdef DEBUG
    /* Print information about task */
    snprintf(str, sizeof(str), "stack array base: %04X\r\n", (uint16_t)stack);
    uart1_send_str((uint8_t *)str, 0);

    snprintf(str, sizeof(str), "new_task->stackbase: %04X\r\n", (uint16_t)new_task->stackbase);
    uart1_send_str((uint8_t *)str, 0);

    snprintf(str, sizeof(str), "Stack Size: %d Bytes\r\n", stack_size);
    uart1_send_str((uint8_t *)str, 0);

    snprintf(str, sizeof(str), "new_task->taskSP: %04X\r\n", (uint16_t)new_task->taskSP);
    uart1_send_str((uint8_t *)str, 0);

    snprintf(str, sizeof(str), "Task count: %d\r\n", adk_task_count);
    uart1_send_str((uint8_t *)str, 0);
#endif

    return 0;
}

/*
 * This function starts the schedular
 *
 * @param idle_hook: Function pointer to the function to be executed
 * By idle function. This function can put MCU in sleep or can serve
 * Watchdog, if enabled.
 *  
 */
void adk_start(adk_idle_task_hook_t idle_hook)
{
    /* Set idle hook function */
    adk_idle_task_hook = idle_hook;

    /* Set idx to 0 which is idle task */
    adk_current_task_idx = 0;

    /* Set correct pointer in currentTCB */
    currentTCB = (uint16_t *)&adk_tasks[adk_current_task_idx];

    /* Enable the interrupts */
    __enable_interrupt();

    /* Start the timer */
    timerA_start();

    /* Switch in the first task */

    /* Set the stack pointer to top of stack for current task
     * which is idle task.
     */
    __asm__ __volatile__(
        "mov.w %0,R12 \n\t \
         mov.w @R12,R1"
        : "=m"(currentTCB)
        :);

    /* 
     * Pop all registers.
     * at this point all registers are set to default value 
     * Once all registers are popped out, only SR and PC are
     * remaining on stack.
     * 
     * Issue an RETI instruction which will pop SR and then set
     * PC to the start of the function, just like if it is 
     * returning from an interrupt.
     * 
     */
    asm volatile("pop	r15						\n\t"
                 "pop	r14						\n\t"
                 "pop	r13						\n\t"
                 "pop	r12						\n\t"
                 "pop	r11						\n\t"
                 "pop	r10						\n\t"
                 "pop	r9						\n\t"
                 "pop	r8						\n\t"
                 "pop	r7						\n\t"
                 "pop	r6						\n\t"
                 "pop	r5						\n\t"
                 "pop	r4						\n\t"
                 "bic	#(0xf0),0(r1)			\n\t"
                 "reti							\n\t");

    /* Should not reach here at all */
    while (1)
    {
        ;
    }
}

/*
 * Get ms from the bootup
 * Similar to arduino milis function
 */
uint32_t adk_milis(void)
{
    uint32_t ret;

    __disable_interrupt();
    ret = adk_tick;
    __enable_interrupt();

    /* Scale from tick to actual time in ms */
    ret = ret * (1000 / ADK_CONFIG_TICKS_PER_SECOND);

    return ret;
}