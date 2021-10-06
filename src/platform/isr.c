#include <msp430.h>
#include <stdint.h>

// extern volatile uint32_t pdelay_count;

// __attribute__((interrupt(TIMERA0_VECTOR))) void Timer_A(void)
// {
//     if (pdelay_count > 0)
//         pdelay_count--;
// }