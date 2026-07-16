#ifndef _BSP_USART3_H_
#define    _BSP_USART3_H_

#include "stm32f10x.h"

//#define _SIMULATION_MTK_SOC_

#ifdef _SIMULATION_MTK_SOC_

#define CNTMARK     (0x3fff)
#define ISCR        (1<<14)
#define RXDONE        (1<<15)

extern volatile uint16_t gRxParam;

extern void bsp_usart3_init(uint32_t bound);
extern void handleShell(void);
extern void putChar(char ch);

#else

#define bsp_usart3_init(n)
#define handleShell()

#endif

#endif /* __USART2_H */
