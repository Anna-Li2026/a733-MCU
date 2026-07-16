#ifndef __TRACE_H__
#define __TRACE_H__
#include <stdio.h>
#include "stm32f10x.h"
#include "SysTick.h"


/* define uart1 rx pin for output triger */
#define  TRIGGER_PORT        (GPIOC)
#define  TRIG0               (GPIO_Pin_11)


/* debug trigger io */
#ifdef _TRIGGER_IO_
void TRIGGER_INIT(void);
void TRIG_HIGH(void);
void TRIG_LOW(void);
#else
#define TRIGGER_INIT()
#define TRIG_HIGH()
#define TRIG_LOW()
#endif

/* printf main switchboard */
#ifdef _PRINTF_
#define INF(fmt, ...)        printf(fmt, ##__VA_ARGS__)
#define DBG(fmt, ...)        printf(fmt, ##__VA_ARGS__)
#define WRN(fmt, ...)        printf("\033[33m" fmt "\033[0m", ##__VA_ARGS__)
#define ERR(fmt, ...)        printf("\033[31m" fmt "\033[0m", ##__VA_ARGS__)
void DUMP(const char* name, const void* buff, int length);
#else
#define INF(fmt, ...)
#define DBG(fmt, ...)
#define WRN(fmt, ...)
#define ERR(fmt, ...)
#define DUMP(buf, len)
#endif

/*
 * @brief
 * @details If you want to display debug & procssing MODULE message, define it!
 * @note    If defined, it dependens on <stdio.h>
 */
 
//#define _COMM_DEBUG_ 
#define _BIB_DEBUG_
#define _DHCP_DEBUG_
#define _WIZCHIP_DEBUG_
#define _PROTOCOL_DEBUG_
#define _EEPROM_DEBUG_


#endif 
