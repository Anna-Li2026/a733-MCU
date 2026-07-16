
/**
* @file bsp_usart2.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief
*
* @details usart4 for shell and debug
*
* @date 2022/11/1
*
* @author heyy
*
* @bug
*
* Revisions: v1.0
*
*/
#include "stdio.h" 
#include "SysTick.h"
#include "bsp_uart4.h"
#include "bsp_burnin_board.h"


/**
 * @brief  redirect library function printf to UART4
 * @param
 * @retval
 */
int fputc(int ch, FILE* f)
{
    USART_SendData(UART4, (uint8_t)ch);
    while (USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);    //when USART_FLAG_TC set, tx complete
    return (ch);
}


/**
 * @brief  redirect library function scanf to UART4
 * @param
 * @retval
 */
int fgetc(FILE* f)
{
    while (USART_GetFlagStatus(UART4, USART_FLAG_RXNE) == RESET);  //when USART_FLAG_RXNE set, rx done to read
    return (int)USART_ReceiveData(UART4);
}

static void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* 发送一个字节数据到USART */
	USART_SendData(pUSARTx,ch);
		
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}
/*****************  发送字符串 **********************/
void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
{
	unsigned int k=0;
  do 
  {
      Usart_SendByte( pUSARTx, *(str + k) );
      k++;
  } while(*(str + k)!='\0');
  
  /* 等待发送完成 */
  while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET)
  {};
}
/*****************  发送一个16位数 **********************/
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch)
{
	uint8_t temp_h, temp_l;
	
	/* 取出高八位 */
	temp_h = (ch&0XFF00)>>8;
	/* 取出低八位 */
	temp_l = ch&0XFF;
	
	/* 发送高八位 */
	USART_SendData(pUSARTx,temp_h);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
	
	/* 发送低八位 */
	USART_SendData(pUSARTx,temp_l);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}
/*****************  发送一个字节 **********************/


/**
 * @brief  USART1 GPIO bound 8-N-1
 * @param
 * @retval
 */
void UART4_Init(uint32_t bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    //NVIC_InitTypeDef NVIC_InitStructure; 

    /* config UART4 clock and gpio */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* Configure UART4 Tx (PC.10) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

#ifndef _TRIGGER_IO_
    /* Configure UART4 Rx (PC.11) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

    //    /* Enable UART4 rx interrupt */
    //    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    //    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    //    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    //    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    //    NVIC_Init(&NVIC_InitStructure);

        /* USART4 mode config */
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

#ifdef _TRIGGER_IO_
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
#else
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
#endif

    USART_Init(UART4, &USART_InitStructure);

    //USART_ClearFlag(UART4, USART_FLAG_TC);
    //USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);                      //enable usart4 rx interrupt
    USART_Cmd(UART4, ENABLE);                                            //enable usart4
}



////Carriage return '\r'(0x0d), New line '\n'(0x0a) 
//void USART1_IRQHandler(void)
//{
//    uint8_t ch;

//    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {              //rx interrupt
//        
//        USART_ClearITPendingBit(USART1, USART_IT_RXNE);                  //clear pending
//        ch = USART_ReceiveData(USART1);                                    //get rx data
//        
//        shell(ch);
//        
//    } 
//}

/*********************************************END OF FILE**********************/
