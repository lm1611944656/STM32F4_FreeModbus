#ifndef __LED_H__
#define __LED_H__

#include "stm32f4xx.h" 
void led_init(void);//≥ı ºªØ

#define LED0 PFout(9)
#define LED1 PFout(10)
#define LED2 PEout(13)
#define LED3 PEout(14)

#endif
