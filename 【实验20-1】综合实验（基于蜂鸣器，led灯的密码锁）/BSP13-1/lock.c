#include "lock.h"
#include "delay.h"

// 门锁继电器引脚 PC8（青软遨游100）
void LOCK_Init(void)
{
    GPIO_InitTypeDef gpio;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_8;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &gpio);
    GPIO_ResetBits(GPIOC, GPIO_Pin_8);
}

void Lock_Open(void)
{
    GPIO_SetBits(GPIOC, GPIO_Pin_8);
}

void Lock_Close(void)
{
    GPIO_ResetBits(GPIOC, GPIO_Pin_8);
}