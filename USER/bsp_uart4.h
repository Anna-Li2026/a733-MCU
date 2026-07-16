#ifndef __USART1_H
#define    __USART1_H

#include "stm32f10x.h"

void UART4_Init(uint32_t bound);
void Usart_SendString( USART_TypeDef * pUSARTx, char *str);
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch);

#endif /* __USART1_H */
