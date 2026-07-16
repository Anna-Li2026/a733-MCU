#ifndef _BSP_INTER_XFER_H_
#define  _BSP_INTER_XFER_H_

#include "stm32f10x.h"

void bsp_usart1_init(void);
void xfer_time_handler(void);
int usart1_tx_request(const void* buffer, uint32_t length);
int usart1_rx_request(void* buffer, uint32_t length, int timeout);

#define USART_REC_LEN 200
#define UART1_FIFO_MAXSIZE 40

typedef struct
{
	uint16_t USART_RX_STA;
	uint8_t USART_RX_BUF[USART_REC_LEN];
}UART1_FIFO_NODE;

void UART1_FIFO_GET_NEXT_FRONT(void);
void UART1_FIFO_GET_NEXT_REAR(void);
int GET_UART1_FIFO_Numvalid(void);
int UART1_FIFO_IS_FULL(void);
int UART1_FIFO_IS_EMPTY(void);


extern uint8_t flagg;
extern UART1_FIFO_NODE UART1_FIFO_QUENE[UART1_FIFO_MAXSIZE];

extern uint8_t UART1_FRONT;
extern uint8_t UART1_REAR;

#endif /* __USART2_H */
