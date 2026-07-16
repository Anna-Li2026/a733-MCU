
/**
* @file bsp_usart2.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief
*
* @details usart2 for communication between stm32 and mt6757
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
#include "SysTick.h"
#include "trace.h"
#include "bsp_usart2.h"
#include "socket.h"
#include "protocol.h"


#ifdef _COMM_DEBUG_
#define COMM_INF(fmt, ...)   INF(fmt, ##__VA_ARGS__)
#define COMM_DBG(fmt, ...)   DBG(fmt, ##__VA_ARGS__)
#define COMM_WRN(fmt, ...)   WRN(fmt, ##__VA_ARGS__)
#define COMM_ERR(fmt, ...)   ERR(fmt, ##__VA_ARGS__)
#define COMM_DUMP(buf,len)   DUMP("COMM", buf, len)
#else
#define COMM_INF(fmt, ...)
#define COMM_DBG(fmt, ...)
#define COMM_WRN(fmt, ...)
#define COMM_ERR(fmt, ...)
#define COMM_DUMP(buf,len)
#endif


/**
 * @brief RXIE enable
 * @param
 * @param
 * @retval
 */
static void usart2_nvic_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable USARTy interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/**
 * @brief  usart2 gpio bound 8-N-1
 * @param  bound 92600bps, 2 bit stop, no parity, no hardware flow control
 * @param  tx_dma_buf, the dma buffer for usart2 tx
 * @retval dma_length, the dma length for usart2 tx
 */
static void usart2_hw_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* config USART2, GPIOA, DMA1 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* Configure USART2 Tx (PA.02) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART2 Rx (PA.03) as pull-up input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART2 mode config */
    USART_InitStructure.USART_BaudRate = INTER_COMM_BOUND;
    USART_InitStructure.USART_WordLength = INTER_COMM_DATA_BITS;
    USART_InitStructure.USART_StopBits = INTER_COMM_STOP_BITS;
    USART_InitStructure.USART_Parity = INTER_COMM_PARITY_BITS;
    USART_InitStructure.USART_HardwareFlowControl = INTER_FLOW_CTRL;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                    //TX & RX enable
    USART_Init(USART2, &USART_InitStructure);

    USART_ClearFlag(USART2, USART_FLAG_TC);                                            //clear TC 
    USART_ITConfig(USART2, USART_IT_ERR, ENABLE);                                     //Error interrupt(Frame error, noise error, overrun error)

    USART_Cmd(USART2, ENABLE);
}


/**
  * @brief  usart2 rx/tx dma
  * @param  rx/tx cache
  * @param
  * @retval
  */
static void usart2_dma_init(void)
{
    DMA_InitTypeDef DMA_InitStructure;

    /* dma1 clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_Cmd(DMA1_Channel6, DISABLE);
    DMA_Cmd(DMA1_Channel7, DISABLE);

    /* dma1 channel6/channel7 for usart2  */
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (USART2->DR);                //peripheral base address        
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                   //peripheral base address is fixed
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                            //memory address auto increment    
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;            //peripheral transfer unit is byte
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;                    //memory unit is byte
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                              //medium priority
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                       //disable memory to memory tranfers 
    DMA_InitStructure.DMA_MemoryBaseAddr = 0;
    DMA_InitStructure.DMA_BufferSize = 0;

    /* dma1 channel6 for usart2 rx */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);

    /* dma1 channel7 for usart2 tx */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure);                                     //dma1 channel7 for usart2 tx     

#ifdef _DMA1_Ch6_RX_INT_
    NVIC_InitTypeDef NVIC_InitStructure;
    /* dma1 channle6 interrupt for usart2 rx TC pending */
    DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);                                  //set DMA_CCR3 of TCIE enable transfer complete interrupt
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

    /* enable dma1 6&7 to kick usart2 rx dma */
    DMA_Cmd(DMA1_Channel6, ENABLE);
    DMA_Cmd(DMA1_Channel7, ENABLE);

    /* show register */
    COMM_DBG("USART2 SR %x, CR1 %x, CR2 %x, CR3 %x\r\n", USART2->SR, USART2->CR1, USART2->CR2, USART2->CR3);
    COMM_DBG("DMA1_Channel6 of USART2_RX : ISR %x, IFCR %x, CCR %x, CNDTR %x, CPAR %x, CMAR %x\r\n", \
             DMA1->ISR, DMA1->IFCR, DMA1_Channel6->CCR, DMA1_Channel6->CNDTR, DMA1_Channel6->CPAR, DMA1_Channel6->CMAR);
    COMM_DBG("DMA1_Channel7 of USART2_TX : ISR %x, IFCR %x, CCR %x, CNDTR %x, CPAR %x, CMAR %x\r\n", \
             DMA1->ISR, DMA1->IFCR, DMA1_Channel7->CCR, DMA1_Channel7->CNDTR, DMA1_Channel7->CPAR, DMA1_Channel7->CMAR);
}


/**
  * @brief  USART2->DR DMA rx_length bytes to rx_buf by DMA1_Channel_6
  * @param    rx_buf, request dma buffer
  * @param    rx_length, request dma length
  * @retval
  */
uint32_t usart2_rx_request(void* buffer, uint32_t length)
{
    DMA_Cmd(DMA1_Channel6, DISABLE);                                                 //disable dma1 channel 6 to config    
    DMA_SetCurrMemoryBaseAddr(DMA1_Channel6, (uint32_t)buffer);                         //tranfer buffer
    DMA_SetCurrDataCounter(DMA1_Channel6, length & 0xffff);                             //tranfer size                                       
    DMA_Cmd(DMA1_Channel6, ENABLE);                                                 //enable dma1 channel 6                
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);                                     //kick start usart2 rx dma

    while (DMA_GetFlagStatus(DMA1_FLAG_TC6) == RESET);                                 //dma TC pending
    DMA_ClearFlag(DMA1_FLAG_TC6);                                                     //clear

    return (length - DMA_GetCurrDataCounter(DMA1_Channel6));
}


/**
 * @brief  tx_buf dam tx_length bytes to USART2->DR by DMA1_Channel_7
 * @param    tx_buf, request dma buffer
 * @param    tx_length,  really dma length
 * @retval
 */
uint32_t usart2_tx_request(const void* buffer, uint32_t length)
{
    DMA_Cmd(DMA1_Channel7, DISABLE);                                                //disable dma1 channel 7 to config
    DMA_SetCurrMemoryBaseAddr(DMA1_Channel7, (uint32_t)buffer);                        //tx buffer
    DMA_SetCurrDataCounter(DMA1_Channel7, length & 0xffff);                            //tx length
    DMA_Cmd(DMA1_Channel7, ENABLE);                                                //enable dma1 channel 7                

    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);                                    //kick start usart2 tx dma

    while (DMA_GetFlagStatus(DMA1_FLAG_TC7) == RESET);                                //dma TC
    DMA_ClearFlag(DMA1_FLAG_TC7);                                                    //clear

    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);

    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);                    //uart2 TC
    USART_ClearFlag(USART2, USART_FLAG_TC);                                            //clear

    //COMM_DBG("%s DMA1_Channel7 : ISR %8x, CNDTR %4x, CMAR %8x. USART2 SR %4x\r\n", __FUNCTION__, DMA1->ISR, DMA1_Channel7->CNDTR, DMA1_Channel7->CMAR, USART2->SR);

    return (length - DMA_GetCurrDataCounter(DMA1_Channel7));
}


/**
 * @brief
 * @param
 * @retval
 */
void bsp_usart2_init(void)
{
    usart2_nvic_init();
    usart2_hw_init();
    usart2_dma_init();
}


/**
 * @brief  DMA1 Channel6 ISR : DMA_ISR of TCIF indicates that usart2 rx data has been DMAed to buffer
 * @param
 * @retval
 */
void DMA1_Channel6_IRQHandler(void)
{
    TRIG_LOW(TRIG1);

    if (DMA_GetITStatus(DMA1_IT_TC6) != RESET)
    {                                        //check DMA_ISR of TCIF
        DMA_ClearITPendingBit(DMA1_IT_TC6);                                            //clear DMA_IFCR of CTCIF

    }

    TRIG_HIGH(TRIG1);
}



/**
* @brief  USART2 rx ISR. Line Terminator : DOS Terminators - CR/LF
  * @param
  * @retval
  */
void USART2_IRQHandler(void)
{
    TRIG_LOW(TRIG2);

    if (USART_GetITStatus(USART2, USART_IT_ORE) != RESET)
    {
        USART_ReceiveData(USART2); //clear overrun pending

        COMM_ERR("Overrun!");
    }
    else if (USART_GetITStatus(USART2, USART_IT_FE) != RESET)
    {
        USART_ReceiveData(USART2); //clear fe pending

        COMM_ERR("Framing!");
    }
    else if (USART_GetITStatus(USART2, USART_IT_NE) != RESET)
    {
        USART_ReceiveData(USART2); //clear ne pending    

        COMM_ERR("Noise!");
    }

    TRIG_HIGH(TRIG2);
}

/*********************************************END OF FILE**********************/
