
/**
* @file trace.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief main
*
* @details define the debug trace method
*
* @date 2022/9/22
*
* @author heyy
*
* @bug
*
* Revisions: v1.0
*
*/
#include "trace.h"

#ifdef _TRIGGER_IO_

/**
 * @brief gpiog for trigger debug
 */
void TRIGGER_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_SetBits(TRIGGER_PORT, TRIG0);
    GPIO_InitStructure.GPIO_Pin = TRIG0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TRIGGER_PORT, &GPIO_InitStructure);
}


/**
 * @brief trig high
 */
void TRIG_HIGH(void)
{
    GPIO_SetBits(TRIGGER_PORT, TRIG0);
}


/**
 * @brief trig low
 */
void TRIG_LOW(void)
{
    GPIO_ResetBits(TRIGGER_PORT, TRIG0);
}

#endif


#ifdef _PRINTF_
/**
 * @brief dump buff data
 */
void DUMP(const char* name, const void* buff, int length)
{
    int i;
    uint8_t* ptr = (uint8_t*)buff;

    printf("\033[33m%s %d hex:\033[0m", name, length);
	  for (i = 0; i < length; ++i)
    {
        if((i&15) == 0) printf("\n");
        printf("%02x ", ptr[i]);
    }
		
		printf("\n");
}
#endif
