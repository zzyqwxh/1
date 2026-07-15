#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "nfc_pn532.h"

// 仅定义一次接收缓冲区，删除重复定义
uint8_t PN532_RxBuf[64];
uint16_t PN532_RxBufLen = 0;

#define PN532_USART USART1
#define LEN_ACK 6       //ACK帧的长度
#define INDEX_LEN 3     //LEN字节在命令中的偏移
#define INDEX_TFI 5     //TFI字节在命令中的偏移

#define CMD_SAMConfiguration 0x14
#define CMD_InListPassiveTarget 0x4A
#define CMD_InDataExchange 0x40

#define PN532_DELAY_MS(x) delay_ms(x)

//NFC读写尝试次数，测试改为10次方便排查故障，0为永久阻塞
uint32_t MAX_TRY = 10;

uint8_t UID[4];
uint8_t UID_backup[4];
uint8_t KEY_A[6]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t KEY_B[6]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

int8_t PN532_WakeUp(void);
int8_t PN532_InListPassiveTarget(void);
int8_t PN532_PsdVerifyKeyA(uint8_t *pKeyA);
int8_t PN532_Read(uint8_t block, uint8_t* buf);
int8_t PN532_Write(uint8_t block, uint8_t* buf);
int8_t PN532_SAMConfiguration(uint8_t mode);

void NFC_Init(uint32_t baud)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(PN532_USART, &USART_InitStructure);

    USART_ITConfig(PN532_USART, USART_IT_RXNE, ENABLE);
    USART_Cmd(PN532_USART, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void PN532_SendData(uint8_t *data, uint8_t length)
{
    uint8_t i = 0;
    for(i=0; i<length; i++)
    {
        USART_SendData(PN532_USART, data[i]);
        while(USART_GetFlagStatus(PN532_USART, USART_FLAG_TXE) == RESET);
    }
}

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        if(PN532_RxBufLen < 64)
        {
            PN532_RxBuf[PN532_RxBufLen++] = USART_ReceiveData(USART1);
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

int8_t PN532_CheckRxBuf(void)
{
    uint8_t i = 0;
    uint8_t len = 0;
    uint8_t temp = 0;

    if(PN532_RxBufLen < (LEN_ACK + INDEX_TFI + 4))
    {
        return -1;
    }
    len = PN532_RxBuf[LEN_ACK + INDEX_LEN];
    temp = 0x100 - len;
    if(temp != PN532_RxBuf[LEN_ACK + INDEX_LEN + 1])
    {
        return -1;
    }
    temp = 0;
    for(i=0; i<len; i++)
    {
        temp += PN532_RxBuf[LEN_ACK + INDEX_TFI + i];
    }
    temp = 0x100 - temp;
    if(temp != PN532_RxBuf[LEN_ACK + INDEX_TFI + len])
    {
        return -1;
    }
    return 0;
}

uint8_t PN532_CheckSum(uint8_t *data)
{
    uint8_t i = 0;
    uint8_t temp = 0;
    for(i=0; i<data[INDEX_LEN]; i++)
    {
        temp += data[INDEX_TFI + i];
    }
    return 0x100 - temp;
}

int8_t NFC_WakeUp(void)
{
    uint32_t i = 0;
    uint8_t data[14] = {0};
    data[0] = 0x55;
    data[1] = 0x55;

    while(1)
    {
        PN532_SendData(data, sizeof(data));
        if(PN532_SAMConfiguration(1) == 0)
        {
            printf("NFC WakeUP OK\r\n");
            break;
        }
        PN532_DELAY_MS(100);
        i++;
        if(MAX_TRY != 0 && i>MAX_TRY)
        {
            printf("NFC WakeUP TimeOut\r\n");
            return -1;
        }
    }
    return 0;
}

int8_t PN532_SAMConfiguration(uint8_t mode)
{
    uint8_t data[10] = {0};
    int8_t ret = 0;

    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0xFF;
    data[3] = 0x03;
    data[4] = 0x100 - data[3];
    data[5] = 0xD4;
    data[6] = CMD_SAMConfiguration;
    data[7] = mode;
    data[8] = PN532_CheckSum(data);
    data[9] = 0x00;

    PN532_SendData(data, sizeof(data));
    PN532_DELAY_MS(200);

    if(PN532_CheckRxBuf()<0)
    {
        ret = -1;
        goto _END;
    }
    ret = 0;

_END:
    PN532_ClearRxBuf();
    return ret;
}

int8_t PN532_InListPassiveTarget(void)
{
    uint8_t data[11] = {0};
    int8_t ret = 0;

    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0xFF;
    data[3] = 0x04;
    data[4] = 0x100 - data[3];
    data[5] = 0xD4;
    data[6] = CMD_InListPassiveTarget;
    data[7] = 0x01;
    data[8] = 0x00;
    data[9] = PN532_CheckSum(data);
    data[10] = 0x00;

    PN532_SendData(data, sizeof(data));
    PN532_DELAY_MS(200);

    if(PN532_CheckRxBuf()<0)
    {
        ret = -1;
        goto _END;
    }
    if(PN532_RxBuf[LEN_ACK + INDEX_TFI + 2] != 1)
    {
        ret = 0;
        goto _END;
    }
    UID[0] = PN532_RxBuf[LEN_ACK + INDEX_TFI + 8];
    UID[1] = PN532_RxBuf[LEN_ACK + INDEX_TFI + 9];
    UID[2] = PN532_RxBuf[LEN_ACK + INDEX_TFI + 10];
    UID[3] = PN532_RxBuf[LEN_ACK + INDEX_TFI + 11];
    ret = 1;

_END:
    PN532_ClearRxBuf();
    return ret;
}

int8_t PN532_PsdVerifyKeyA(uint8_t *pKeyA)
{
    uint8_t data[22] = {0};
    int8_t ret = 0;

    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0xFF;
    data[3] = 0x0F;
    data[4] = 0x100 - data[3];
    data[5] = 0xD4;
    data[6] = CMD_InDataExchange;
    data[7] = 0x01;
    data[8] = 0x60;
    data[9] = 0x03;

    data[10] = pKeyA[0];
    data[11] = pKeyA[1];
    data[12] = pKeyA[2];
    data[13] = pKeyA[3];
    data[14] = pKeyA[4];
    data[15] = pKeyA[5];

    data[16] = UID[0];
    data[17] = UID[1];
    data[18] = UID[2];
    data[19] = UID[3];
    data[20] = PN532_CheckSum(data);
    data[21] = 0x00;

    PN532_SendData(data, sizeof(data));
    PN532_DELAY_MS(200);

    if(PN532_CheckRxBuf()<0)
    {
        ret = -1;
        goto _END;
    }
    if(PN532_RxBuf[LEN_ACK + INDEX_TFI + 2] != 0)
    {
        ret = -1;
        goto _END;
    }
    ret = 0;
_END:
    PN532_ClearRxBuf();
    return ret;
}

int8_t PN532_Write(uint8_t block, uint8_t* buf)
{
    uint8_t data[28] = {0};
    int8_t ret = 0;

    if(buf == NULL)
    {
        return -1;
    }

    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0xFF;
    data[3] = 0x15;
    data[4] = 0x100 - data[3];
    data[5] = 0xD4;
    data[6] = CMD_InDataExchange;
    data[7] = 0x01;
    data[8] = 0xA0;
    data[9] = block;
    memcpy(data+10, buf, 16);
    data[26] = PN532_CheckSum(data);
    data[27] = 0x00;

    PN532_SendData(data, sizeof(data));
    PN532_DELAY_MS(200);

    if(PN532_CheckRxBuf()<0)
    {
        ret = -1;
        goto _END;
    }
    if(PN532_RxBuf[LEN_ACK + INDEX_TFI + 2] != 0)
    {
        ret = -1;
        goto _END;
    }
    ret = 0;

_END:
    PN532_ClearRxBuf();
    return ret;
}

int8_t PN532_Read(uint8_t block , uint8_t *buf)
{
    uint8_t data[12] = {0};
    int8_t ret = 0;

    if(buf == NULL)
    {
        return -1;
    }

    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0xFF;
    data[3] = 0x05;
    data[4] = 0x100 - data[3];
    data[5] = 0xD4;
    data[6] = 0x40;
    data[7] = 0x01;
    data[8] = 0x30;
    data[9] = block;
    data[10] = PN532_CheckSum(data);
    data[11] = 0x00;

    PN532_SendData(data, sizeof(data));
    PN532_DELAY_MS(200);

    if(PN532_CheckRxBuf()<0)
    {
        ret = -1;
        goto _END;
    }
    if(PN532_RxBuf[LEN_ACK + INDEX_TFI + 2] != 0)
    {
        ret = -1;
        goto _END;
    }

    memcpy(buf, PN532_RxBuf + LEN_ACK + INDEX_TFI + 3, 16);
    ret = 0;

_END:
    PN532_ClearRxBuf();
    return ret;
}

int8_t NFC_Write(uint8_t block, uint8_t* buf)
{
    uint32_t i = 0;
    while(1)
    {
        PN532_DELAY_MS(100);
        i++;
        if(MAX_TRY!=0 && i>MAX_TRY)
        {
            printf("NFC Write TimeOut\r\n");
            return -1;
        }
        if(PN532_InListPassiveTarget()<=0)
        {
            printf("未检测到卡片，重新放卡\r\n");
            continue;
        }
        if(PN532_PsdVerifyKeyA(KEY_A) < 0)
        {
            printf("密钥验证失败\r\n");
            continue;
        }
        if(PN532_Write(block , buf)<0)
        {
            printf("块写入失败\r\n");
            continue;
        }
        break;
    }
    return 0;
}

int8_t NFC_Read(uint8_t block, uint8_t* buf)
{
    uint32_t i = 0;
    while(1)
    {
        PN532_DELAY_MS(100);
        i++;
        if(MAX_TRY != 0 && i>MAX_TRY)
        {
            printf("NFC Read TimeOut\r\n");
            return -1;
        }
        if(PN532_InListPassiveTarget() <= 0)
        {
            printf("未检测到卡片，重新放卡\r\n");
            continue;
        }
        if(PN532_PsdVerifyKeyA(KEY_A) < 0)
        {
            printf("密钥验证失败\r\n");
            continue;
        }
        if(PN532_Read(block , buf)<0)
        {
            printf("块读取失败\r\n");
            continue;
        }
        break;
    }
    return 0;
}

void PN532_ClearRxBuf(void)
{
    memset(PN532_RxBuf, 0, sizeof(PN532_RxBuf));
    PN532_RxBufLen = 0;
}