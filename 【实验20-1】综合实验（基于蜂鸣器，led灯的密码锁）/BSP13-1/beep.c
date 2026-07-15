#include "beep.h"
#include "delay.h"

void BEEP_Init(void)
{
    GPIO_InitTypeDef gpio;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = GPIO_Pin_8;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_25MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    // 上电默认高电平，三极管截止，蜂鸣器静音，解决长鸣
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
    GPIO_Init(GPIOA, &gpio);
}

// 密码正确短鸣：拉低PA8发声，延时后拉高静音
void Beep_Short(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    delay_ms(100);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
}

// 密码错误长鸣：拉低PA8发声，延时后拉高静音
void Beep_Long(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    delay_ms(1000);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
}