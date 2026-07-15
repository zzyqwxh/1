#ifndef __KEY_H
#define __KEY_H
#include "stm32f4xx.h"

// 按键对应引脚
#define KEY1_PIN    GPIO_Pin_0
#define KEY1_PORT   GPIOA

#define KEY2_PIN    GPIO_Pin_13
#define KEY2_PORT   GPIOC

#define KEY3_PIN    GPIO_Pin_6
#define KEY3_PORT   GPIOC

#define KEY4_PIN    GPIO_Pin_7
#define KEY4_PORT   GPIOC

void KEY_Init(void);
uint8_t KEY_Scan(void); // 返回1/2/3/4，无按键0
#endif