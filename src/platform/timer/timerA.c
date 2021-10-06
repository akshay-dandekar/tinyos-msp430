#include <msp430.h>
#include <stdint.h>
#include "timerA.h"

void timerA_start(void)
{
	TACTL |= (1 << 4);
}

void timerA_stop(void)
{
	TACTL &= ~(1 << 4);
}

void timerA_init(uint16_t ticks_per_second)
{

	/* Stop timer if running */
	TACTL &= ~(1 << 4);

	/* Clear TACLR by writing 1 to it */
	TACTL |= (1 << 2);

	/* Timer A clock source selected as ACLK */
	TACTL |= (1 << 8);

	/*Clock divide by 1 as frequency is 32768*/
	TACTL &= ~((1 << 7) | (1 << 6));

	/* Count for interrupt every 10ms */
	TACCR0 = 32768 / ticks_per_second;

	/* TACCR0 interrupt enabled */
	TACCTL0 = (1 << 4);
}
