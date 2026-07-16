#ifndef __USART1_H
#define    __USART1_H

#include "stm32f10x.h"

//#define USART1_REC_LEN 200                        //每行最大接收字节数200
//extern u8  USART1_RX_BUF[USART1_REC_LEN];         //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
//extern u16 USART1_RX_STA;                         //接收状态标记

void USART1_Init(uint32_t bound);


#endif /* __USART1_H */
