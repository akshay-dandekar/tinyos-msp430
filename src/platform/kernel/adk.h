#ifndef _ADK_H_
#define _ADK_H_
#include <stdint.h>
#include "adk_config.h"

#define ADK_MS_TO_TICKS(x) (((uint32_t)x * (uint32_t)ADK_CONFIG_TICKS_PER_SECOND) / 1000UL)

typedef void (*adk_idle_task_hook_t)(void);
typedef void (*TaskFunction_t)(void);

void adk_init(void);

int adk_task_add(TaskFunction_t run, uint8_t *stack, uint16_t stack_size);

void adk_yield(void);

void adk_start(adk_idle_task_hook_t idle_hook);

void adk_delay(uint32_t ticks);

uint32_t adk_milis(void);

#endif /* _ADK_H_ */