#ifndef __LED_H
#define __LED_H
#include "stm32f4xx.h"

#define LED1_PIN GPIO_Pin_5
#define LED2_PIN GPIO_Pin_6
#define LED3_PIN GPIO_Pin_7
#define LED_PORT GPIOB

void LED_Init(void);
void LED_AllOff(void);
void LED_AllOn(void);
void LED_Flash(uint16_t t);
#endif