
/**
* @file bsp_usart2.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief
*
* @details usart3 polling simulate mt6757
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
#include <string.h>
#include "trace.h"
#include "SysTick.h"
#include "bsp_usart3.h"
#include "protocol.h"
#include "nr_micro_shell.h"

#ifdef _SIMULATION_MTK_SOC_


extern uint8_t gDATABUF[];
volatile uint16_t gRxParam = 0;

//backup receive ITVL buffer
static uint8_t gRXITVL[512] __attribute__((aligned(8)));


static void usart3_hw_init(uint32_t bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* Configure USART3 Tx (PB.10) --> PA03 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* Configure USART3 Rx (PB.11) --> PA02 as pull up input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    /* USART3 mode config */
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_2;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    USART_ClearFlag(USART3, USART_FLAG_TC);                                                //clear TC

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);                                        //ORE/RXNE interrupt. Must interrupt receive!        
    USART_ITConfig(USART3, USART_IT_ERR, ENABLE);                                        //Error interrupt(Frame error, noise error, overrun error)

    USART_Cmd(USART3, ENABLE);

    //INF("USART3 SR %x, DR %x, BRR %x, CR1 %x, CR2 %x, CR3 %x, USART_GTPR %x\r\n",\
        USART3->SR, USART3->DR, USART3->BRR, USART3->CR1, USART3->CR2, USART3->CR3, USART3->GTPR);

}


/**
 * @brief
 * @param
 * @retval
 */
 //__attribute__((unused)) static uint16_t getChar(void) 
 //{

 //    while(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET);    //rx byte pending
 //    return USART_ReceiveData(USART3);
 //}


  /**
   * @brief
   * @param
   * @retval
   */
void putChar(char ch)
{
    USART_SendData(USART3, ch);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);  //when USART_FLAG_TC set, tx complete    
}


static void usart3_nvic_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

//The usart receive must be interrupted, otherwise it's prone to overflow
void USART3_IRQHandler(void)
{
    char ch;

    TRIG_LOW(TRIG4);

    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        ch = USART_ReceiveData(USART3);

        if ((gRxParam & ISCR) == 0)
        {
            if (ch == '\r')
            {
                gRxParam |= ISCR;
            }

            gDATABUF[CNTMARK & gRxParam++] = ch;
            if ((gRxParam & CNTMARK) > (MAX_TRANSMISSION_LENGTH - 1))
            {
                gRxParam = 0;
            }
        }
        else
        {    /* forced end flag : "\r\n" */
            gDATABUF[CNTMARK & gRxParam] = '\n';
            gRxParam |= RXDONE;
        }
    }
    else if (USART_GetITStatus(USART3, USART_IT_ORE) != RESET)
    {
        USART_ReceiveData(USART3);
        gRxParam = 0;

        ERR("Overrun!");
    }
    else if (USART_GetITStatus(USART3, USART_IT_FE) != RESET)
    {
        USART_ReceiveData(USART3);
        gRxParam = 0;

        ERR("Framing!");
    }
    else if (USART_GetITStatus(USART3, USART_IT_NE) != RESET)
    {
        USART_ReceiveData(USART3);
        gRxParam = 0;

        ERR("Noise!");
    }

    TRIG_HIGH(TRIG4);
}


#ifdef _USART3_DMA_

static uint32_t usart3_rx_request(void* buffer, uint32_t length)
{
    DMA_Cmd(DMA1_Channel3, DISABLE);                                                 //disable dma1 channel 3 to config    
    DMA_SetCurrMemoryBaseAddr(DMA1_Channel3, (uint32_t)buffer);                         //rx buffer
    DMA_SetCurrDataCounter(DMA1_Channel3, length);                                     //rx length
    DMA_Cmd(DMA1_Channel3, ENABLE);                                                 //enable dma1 channel 3    
    USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);

    while (DMA_GetFlagStatus(DMA1_FLAG_TC3) == RESET);                                 //dma TC
    DMA_ClearFlag(DMA1_FLAG_TC3);                                                     //clear

    return (length - DMA_GetCurrDataCounter(DMA1_Channel3));
}


static uint32_t usart3_tx_request(void* buffer, uint32_t length)
{
    DMA_Cmd(DMA1_Channel2, DISABLE);                                                //disable dma1 channel 2 to config
    DMA_SetCurrMemoryBaseAddr(DMA1_Channel2, (uint32_t)buffer);                        //tx buffer
    DMA_SetCurrDataCounter(DMA1_Channel2, length);                                    //tx length
    DMA_Cmd(DMA1_Channel2, ENABLE);                                                //enable dma1 channel 2
    USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);

    while (DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET);                                //dma TC
    DMA_ClearFlag(DMA1_FLAG_TC2);                                                    //clear

    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);

    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);                    //uart2 TC
    USART_ClearFlag(USART3, USART_FLAG_TC);                                            //clear

    return (length - DMA_GetCurrDataCounter(DMA1_Channel2));
}


/**
* @brief  DMA1 Channel3 ISR : DMA_ISR of TCIF indicates that usart3 rx data has been DMAed to buffer
  * @param
  * @retval
  */
void DMA1_Channel3_IRQHandler(void)
{
    TRIG_LOW(TRIG3);

    if (DMA_GetITStatus(DMA1_IT_TC3) != RESET)
    {                                        //check DMA_ISR of TCIF
        DMA_ClearITPendingBit(DMA1_IT_TC3);                                            //clear DMA_IFCR of CTCIF

    }

    TRIG_HIGH(TRIG3);
}


static void usart3_dma_init(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* dma1 clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* dma1 channel2/channel3 for usart3  */
    DMA_Cmd(DMA1_Channel2, DISABLE);                                                //must disable dma1 channel to config
    DMA_Cmd(DMA1_Channel3, DISABLE);

    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (USART3->DR);                //peripheral base address        

    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                 //peripheral base address is fixed
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                            //memory address auto increment    
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;            //peripheral transfer unit is byte
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;                     //memory unit is byte
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                              //medium priority
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                    //disable memory to memory tranfers 
    DMA_InitStructure.DMA_MemoryBaseAddr = 0;                                        //defaule max
    DMA_InitStructure.DMA_BufferSize = 0;                                            //default DMA1_Channel3 occure rx

    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                                  //read peripheral
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);                                     //dma1 channel3 for usart3 rx     

    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                                //write perihperalSRC
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);                                    //dma1 channel2 for usart3 tx

    /* dma1 channle3 interrupt for usart3 rx TC pending */
    DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);                                  //set DMA_CCR3 of TCIE enable transfer complete interrupt
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* enable dma1 2&3 to kick usart3 rx dma request */
    DMA_Cmd(DMA1_Channel3, ENABLE);
    DMA_Cmd(DMA1_Channel2, ENABLE);
    USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);


    //usart3_rx_request(gITVL, sizeof(ITVL_t));

    /* show register */
    //printf("USART3 SR %x, CR1 %x, CR2 %x, CR3 %x\r\n", USART3->SR, USART3->CR1, USART3->CR2, USART3->CR3);    
    //printf("DMA1_Channel2 USART3 TX : ISR %x, IFCR %x, CCR %x, CNDTR %x, CPAR %x, CMAR %x\r\n", \
    //DMA1->ISR, DMA1->IFCR, DMA1_Channel2->CCR, DMA1_Channel2->CNDTR, DMA1_Channel2->CPAR, DMA1_Channel2->CMAR);
    //printf("DMA1_Channel3 USART3 RX : ISR %x, IFCR %x, CCR %x, CNDTR %x, CPAR %x, CMAR %x\r\n", \
    //DMA1->ISR, DMA1->IFCR, DMA1_Channel3->CCR, DMA1_Channel3->CNDTR, DMA1_Channel3->CPAR, DMA1_Channel3->CMAR);

}
#endif


/**
 * @brief  USART3 GPIO bound 8-N-1
 * @param  115200, 2 bit stop, no parity, no hardware flow control
 * @retval
 */
void bsp_usart3_init(uint32_t bound)
{
    usart3_nvic_init();
    usart3_hw_init(bound);    //using polling

#ifdef _USART3_DMA_    
    usart3_dma_init();
#endif
}


/**
 * @brief
 * @param
 * @retval
 */
void handleShell(void)
{
    int i;
    ITVL_t* pITVL = (ITVL_t*)gRXITVL;

    shell_init();                                //register shell                                
    while (1)
    {
        if (gRxParam & RXDONE)
        {
            i = ((gRxParam & CNTMARK) > sizeof(gRXITVL)) ? sizeof(gRXITVL) : (gRxParam & CNTMARK);
            memcpy(gRXITVL, gDATABUF, i);
            gRxParam = 0;                         //clear pending

            if (pITVL->wHead == ITVL_HEAD)
            {

                for (i = 0; pITVL->value[i] != '\0'; ++i)
                {
                    WRN("%d ", i);
                    shell(pITVL->value[i]);
                }
            }
        }
    }
}

#endif

/*********************************************END OF FILE**********************/
