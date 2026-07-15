#ifndef __NFC_PN532_H__
#define __NFC_PN532_H__

#include "stm32f4xx.h"

// 统一只保留一组接收缓冲区变量
extern uint8_t PN532_RxBuf[64];
extern uint16_t PN532_RxBufLen;

void PN532_ClearRxBuf(void);
void NFC_Init(uint32_t baud);
int8_t NFC_WakeUp(void);
int8_t NFC_Read(uint8_t block, uint8_t *buf);
int8_t NFC_Write(uint8_t block, uint8_t *buf);

#endif /* __NFC_PN532_H__ */