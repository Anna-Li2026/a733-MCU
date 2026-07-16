
/**
* @file bsp_usart2.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief
*
* @details usart3 for shell and debug
*
* @date 2022/9/2
*
* @author heyy
*
* @bug
*
* Revisions: v1.0
*
*/
#include "bsp_usart1.h"
#include "nr_micro_shell.h"

//#pragma import(__use_no_semihosting_swi)

//uint8_t USART1_RX_BUF[USART1_REC_LEN];    //buffer of receive
//uint16_t USART1_RX_STA=0;                //status of reveiving, bit15 done flag, bit14 flag of '\r', count : bit13~0


 /**
  * @brief  redirect library function printf to usart1
  * @param
  * @retval
  */
int fputc(int ch, FILE* f)
{
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);    //when USART_FLAG_TC set, tx complete        
    return (ch);
}


/**
 * @brief  redirect library function scanf to usart1
 * @param
 * @retval
 */
int fgetc(FILE* f)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);  //when USART_FLAG_RXNE set, rx done to read
    return (int)USART_ReceiveData(USART1);
}


/**
 * @brief  USART1 GPIO bound 8-N-1
 * @param
 * @retval
 */
void USART1_Init(uint32_t bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* config USART1 clock and gpio */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    /* Configure USART1 Tx (PA.09) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* Configure USART1 Rx (PA.10) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Enable USARTy rx interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* USART1 mode config */
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_ClearFlag(USART1, USART_FLAG_TC);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                        //enable usart1 rx interrupt
    USART_Cmd(USART1, ENABLE);                                            //enable usart1

}



//串口1中断服务程序 Carriage return '\r'(0x0d), New line '\n'(0x0a) 
void USART1_IRQHandler(void)
{
    uint8_t ch;

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {                //rx interrupt     

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);                    //clear pending
        ch = USART_ReceiveData(USART1);                                    //get rx data

        shell(ch);

        //if((USART1_RX_STA & (1<<15)) == 0) {
        //    if( USART1_RX_STA & (1<<14) ) {                             //is '\r'
        //        if(ch != '\n') {
        //                USART1_RX_STA=0;                                //error, let's begin
        //        } else {
        //            USART1_RX_STA |= (1<<15);                            //done
        //        }
        //    } else {                                                    //not '\r'
        //        if(ch == '\r') {
        //            USART1_RX_STA |= (1<<14);
        //        } else {
        //            USART1_RX_BUF[USART1_RX_STA&0x3FFFF] = ch;
        //            USART1_RX_STA++;
        //            if(USART1_RX_STA > (USART1_REC_LEN -1))
        //                USART1_RX_STA = 0;                                //error, let's begin
        //        }
        //    }
        //}

    }
}

/*********************************************END OF FILE**********************/
