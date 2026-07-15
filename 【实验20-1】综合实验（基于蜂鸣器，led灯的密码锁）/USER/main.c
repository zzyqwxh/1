#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "oled.h"
#include "beep.h"
#include "lock.h"
#include "key.h"
#include "led.h"
#include "usart.h"

// 自定义4位密码，可自行修改
uint8_t passwd[4] = {1,2,3,1};
uint8_t input_buf[4];
uint8_t input_cnt = 0;

void Show_Input(void)
{
    char buf[20];
    OLED_Clear();
    OLED_ShowString(10,10,"请输入4位密码");
    sprintf(buf,"输入：");
    for(uint8_t i=0;i<input_cnt;i++)
    {
        sprintf(buf+strlen(buf),"%d ",input_buf[i]);
    }
    OLED_ShowString(10,30,buf);
    OLED_ShowString(10,50,"KEY4=确认/清空");
}

int main(void)
{
    uint8_t key_val;
    SystemInit();
    delay_init();
    UART2_Init(115200);
    OLED_Init();
    BEEP_Init();
    LOCK_Init();
    KEY_Init();
    LED_Init();

    LED_AllOff();
    input_cnt = 0;
    memset(input_buf,0,4);
    Show_Input();

    while(1)
    {
        key_val = KEY_Scan();
        if(key_val != 0)
        {
            if(key_val == 4)
            {
                // 确认/清空
                if(input_cnt < 4)
                {
                    // 输入不足4位，清空
                    input_cnt = 0;
                    memset(input_buf,0,4);
                    Show_Input();
                }
                else
                {
                    // 校验密码
                    if(memcmp(input_buf, passwd, 4) == 0)
                    {
                        // 密码正确
                        OLED_Clear();
                        OLED_ShowString(20,20,"解锁成功！");
                        LED_AllOn();
                        Beep_Short();
                        Lock_Open();
                        delay_ms(3000);
                        Lock_Close();
                    }
                    else
                    {
                        // 密码错误
                        OLED_Clear();
                        OLED_ShowString(20,20,"密码错误");
                        LED_Flash(300);
                        Beep_Long();
                        delay_ms(2000);
                    }
                    // 重置输入
                    input_cnt = 0;
                    memset(input_buf,0,4);
                    LED_AllOff();
                    Show_Input();
                }
            }
            else
            {
                // KEY1/2/3输入数字
                if(input_cnt < 4)
                {
                    input_buf[input_cnt++] = key_val;
                    Show_Input();
                }
            }
        }
        delay_ms(50);
    }
}