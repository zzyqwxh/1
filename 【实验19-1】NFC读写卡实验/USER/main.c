#include <stdio.h>
#include "stm32f4xx.h"
#include "nfc_pn532.h"
#include "usart.h"
#include "delay.h"

#define TEST_SIZE 16
uint8_t BufRead[TEST_SIZE] = {0};
uint8_t BufWrite[TEST_SIZE] = {0};

void make_test_data()
{
    uint32_t i = 0;
    for(i=0; i<TEST_SIZE; i++ )
    {
        BufWrite[i] = i;
        printf("0x%02X ", BufWrite[i]);
        if(i%16 == 15)
        {
            printf("\r\n");
        }
    }
}

void check_test_data()
{
    uint32_t i = 0;
    uint8_t data_ok = 1;
    for(i=0; i<TEST_SIZE; i++ )
    {
        if(BufRead[i] != BufWrite[i])
        {
            printf("¶ÁÈ¡Êý¾ÝÒì³££¬µÚ%d×Ö½Ú:0x%02X Ô¤ÆÚ0x%02X\r\n",i,BufRead[i],BufWrite[i]);
            data_ok = 0;
        }
        printf("0x%02X ", BufRead[i]);
        if(i%16 == 15)
        {
            printf("\r\n");
        }
    }
    printf("\r\n");
    if(data_ok == 1)
    {
        printf("TEST OK\r\n");
    }
}

int main(void)
{
    UART2_Init(115200);
    NFC_Init(115200);
    printf("NFC Init OK\r\n");

    // ÅÐ¶Ï»½ÐÑ×´Ì¬£¬Ê§°ÜÍ£»ú
    if(NFC_WakeUp() < 0)
    {
        printf("NFCÄ£¿é»½ÐÑÊ§°Ü£¬Çë¼ì²éÌøÏß¡¢Ó²¼þ½ÓÏß£¡\r\n");
        while(1);
    }

    make_test_data();
    while(1)
    {
        if(NFC_Write(2, BufWrite)<0)
        {
            continue;
        }
        printf("Write OK\r\n");
        delay_ms(5000);

        if(NFC_Read(2, BufRead)<0)
        {
            continue;
        }
        printf("Read OK\r\n");
        check_test_data();
        delay_ms(30000);
    }
}
