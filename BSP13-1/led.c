#include "led.h"
#include "delay.h"

void LED_Init(void)
{
    GPIO_InitTypeDef gpio;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    gpio.GPIO_Pin = LED1_PIN|LED2_PIN|LED3_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(LED_PORT, &gpio);
    GPIO_ResetBits(LED_PORT, LED1_PIN|LED2_PIN|LED3_PIN);
}

void LED_AllOff(void)
{
    GPIO_ResetBits(LED_PORT, LED1_PIN|LED2_PIN|LED3_PIN);
}

void LED_AllOn(void)
{
    GPIO_SetBits(LED_PORT, LED1_PIN|LED2_PIN|LED3_PIN);
}

void LED_Flash(uint16_t t)
{
    uint8_t i;
    for(i=0;i<3;i++)
    {
        LED_AllOn();
        delay_ms(t);
        LED_AllOff();
        delay_ms(t);
    }
}