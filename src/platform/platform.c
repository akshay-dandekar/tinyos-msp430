/* Configuration of clock MCLK and SMCLK to 8MHZ and ACLK to 32768HZ*/
#include <msp430.h>
#include <stdio.h>
#include "platform.h"
#include "platform/gpio/gpio.h"

volatile uint32_t pdelay_count;

void platform_delay(uint32_t ms)
{
	pdelay_count = ms;
	while (pdelay_count > 0)
	{
		__nop();
	}
}

void platform_init(void)
{
	int a, b;

	/* Stop WatchDog Timer */
	WDTCTL = WDTPW | WDTHOLD;

	for (a = 1; a <= 1000; a++)
	{
		for (b = 1; b <= 30; b++)
		{
			_no_operation();
		}
	}
	/*Set in Low frequency mode */
	FLL_CTL0 = 0x00;

	/* select range */
	SCFI0 = 0x00;

	/* wait till LFXT1 in LF mode stabilizes */
	while ((FLL_CTL0 & 0x02) != 0)
		;

	/* wait till DCO is stabilized */
	while ((FLL_CTL0 & 0x01) != 0)
		;

	/* clear OFIFG FLAG */
	IFG1 &= ~(1 << 1);

	/*For SMCLK and MCLK */
	FLL_CTL1 &= ~(1 << 5);

	while ((FLL_CTL0 & 0x08) != 0)
		;

	IFG1 &= ~(1 << 1);

	/* Switch MCLK to XT2 */
	FLL_CTL1 |= (2 << 3);

	/* Switch SMCLk to XT2 */
	FLL_CTL1 |= (1 << 2);
	FLL_CTL1 &= ~(1 << 5);
}
