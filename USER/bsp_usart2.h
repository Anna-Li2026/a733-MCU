#ifndef _BSP_USART2_H_
#define    _BSP_USART2_H_

#include "stm32f10x.h"

void bsp_usart2_init(void);
uint32_t usart2_tx_request(const void* buffer, uint32_t length);
uint32_t usart2_rx_request(void* buffer, uint32_t length);

#endif /* __USART2_H */
