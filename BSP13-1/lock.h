#ifndef __LOCK_H
#define __LOCK_H
#include "stm32f4xx.h"

void LOCK_Init(void);
void Lock_Open(void);
void Lock_Close(void);

#endif