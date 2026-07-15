#include "key.h"
#include "delay.h"

void KEY_Init(void)
{
    GPIO_InitTypeDef gpio_cfg;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);

    // PA0 KEY1
    gpio_cfg.GPIO_Pin = KEY1_PIN;
    gpio_cfg.GPIO_Mode = GPIO_Mode_IN;
    gpio_cfg.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(KEY1_PORT, &gpio_cfg);

    // PC13 PC6 PC7 KEY2/3/4
    gpio_cfg.GPIO_Pin = KEY2_PIN | KEY3_PIN | KEY4_PIN;
    gpio_cfg.GPIO_Mode = GPIO_Mode_IN;
    gpio_cfg.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(KEY2_PORT, &gpio_cfg);
}

uint8_t KEY_Scan(void)
{
    // KEY1 PA0 数字1
    if(GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) == 0)
    {
        delay_ms(20);
        while(GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) == 0);
        delay_ms(20);
        return 1;
    }
    // KEY2 PC13 数字2
    if(GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN) == 0)
    {
        delay_ms(20);
        while(GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN) == 0);
        delay_ms(20);
        return 2;
    }
    // KEY3 PC6 数字3
    if(GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN) == 0)
    {
        delay_ms(20);
        while(GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN) == 0);
        delay_ms(20);
        return 3;
    }
    // KEY4 PC7 确认清空
    if(GPIO_ReadInputDataBit(KEY4_PORT, KEY4_PIN) == 0)
    {
        delay_ms(20);
        while(GPIO_ReadInputDataBit(KEY4_PORT, KEY4_PIN) == 0);
        delay_ms(20);
        return 4;
    }
    return 0;
}