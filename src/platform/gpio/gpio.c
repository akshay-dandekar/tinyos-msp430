#include <msp430.h>
#include "gpio.h"

void gpio_init(void)
{

    /* Set ports according to UART */
	/* Init ports */
	P4DIR &= ~(1 << 1);
	P4DIR |= (1 << 0);
	P4SEL |=  (1 << 1) | (1 << 0);

    /* Set port as output for coil drive P2-1 and P2-0 */
    DRVA_OUT &= ~(1 << DRVA_PIN);
    DRVA_DIR |= (1 << DRVA_PIN);
    DRVB_OUT &= ~(1 << DRVB_PIN);
    DRVB_DIR |= (1 << DRVB_PIN);

    /* Set port as output for 12V */
    CNT12V_OUT &= ~(1 << CNT12V_PIN);
    CNT12V_DIR |= (1 << CNT12V_PIN);

    /* Set port as output for logic gates */
	CNTLG_OUT &= ~(1 << CNTLG_PIN);
	CNTLG_DIR |= (1 << CNTLG_PIN);

    /* Set port as output for debug */
	PULSE_OUT_OUT &= ~(1 << PULSE_OUT_PIN);
	PULSE_OUT_DIR |= (1 << PULSE_OUT_PIN);
}