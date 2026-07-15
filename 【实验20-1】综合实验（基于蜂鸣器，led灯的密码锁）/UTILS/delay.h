#ifndef __DELAY_H
#define __DELAY_H
#include "stm32f4xx.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u16 nus);

#endif