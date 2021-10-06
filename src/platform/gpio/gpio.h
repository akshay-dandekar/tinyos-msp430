#ifndef _PLATFORM_GPIO_H_
#define _PLATFORM_GPIO_H_

#define DRVA_OUT P2OUT
#define DRVA_PIN 1
#define DRVA_DIR P2DIR

#define DRVB_OUT P2OUT
#define DRVB_PIN 0
#define DRVB_DIR P2DIR

#define CNT12V_OUT P2OUT
#define CNT12V_PIN 6
#define CNT12V_DIR P2DIR

#define CNTLG_OUT P1OUT
#define CNTLG_PIN 2
#define CNTLG_DIR P1DIR

#define PULSE_OUT_OUT P3OUT
#define PULSE_OUT_DIR P3DIR
#define PULSE_OUT_SEL P3SEL
#define PULSE_OUT_PIN 7

void gpio_init(void);

#endif /* _PLATFORM_GPIO_H_ */