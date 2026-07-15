#include "delay.h"

void delay_init(void)
{
    // F407 168MHz 简易延时初始化（空函数适配你的代码）
}

void delay_us(u16 nus)
{
    u16 i;
    while(nus--)
    {
        i = 31;
        while(i--);
    }
}

void delay_ms(u16 nms)
{
    u16 i;
    while(nms--)
    {
        i = 33800;
        while(i--);
    }
}