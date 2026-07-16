
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
#include "string.h"
#include "trace.h"
#include "bsp_inter_xfer.h"
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


static int inter_xfer_tick_1s;

UART1_FIFO_NODE UART1_FIFO_QUENE[UART1_FIFO_MAXSIZE];

uint8_t UART1_FRONT=0;
uint8_t UART1_REAR=0;

uint8_t flagg=1;
static int inter_xfer_tick_1s;

void UART1_FIFO_GET_NEXT_FRONT(void)
{
	UART1_FRONT=(UART1_FRONT+1)%UART1_FIFO_MAXSIZE;
}
void UART1_FIFO_GET_NEXT_REAR(void)
{
	UART1_REAR=(UART1_REAR+1)%UART1_FIFO_MAXSIZE;
}
int GET_UART1_FIFO_Numvalid(void)
{
	int a;
	a=(UART1_REAR+UART1_FIFO_MAXSIZE-UART1_FRONT)%UART1_FIFO_MAXSIZE;
	return a;
}
int UART1_FIFO_IS_FULL(void)
{
	if(((UART1_REAR+1)%UART1_FIFO_MAXSIZE)==UART1_FRONT)
		return 1;
	else
		return 0;
}
int UART1_FIFO_IS_EMPTY(void)
{
	if(UART1_REAR==UART1_FRONT)
		return 1;
	else
		return 0;
}

/**
 * @brief 
 * @param
 * @param
 * @retval
 */
void xfer_time_handler(void)
{
    inter_xfer_tick_1s++;
}


/**
 * @brief RXIE enable
 * @param
 * @param
 * @retval
 */
static void usart1_nvic_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable USARTy interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/**
 * @brief  usart1 gpio bound 8-N-1
 * @param  bound 92600bps, 2 bit stop, no parity, no hardware flow control
 * @param  tx_dma_buf, the dma buffer for usart2 tx
 * @retval dma_length, the dma length for usart2 tx
 */
static void usart1_hw_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* config USART1, GPIOA, DMA1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    /* Configure USART1 Tx (PA.09) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART1 Rx (PA.10) as pull-up input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART1 mode config */
    USART_InitStructure.USART_BaudRate = INTER_COMM_BOUND;
    USART_InitStructure.USART_WordLength = INTER_COMM_DATA_BITS;
    USART_InitStructure.USART_StopBits = INTER_COMM_STOP_BITS;
    USART_InitStructure.USART_Parity = INTER_COMM_PARITY_BITS;
    USART_InitStructure.USART_HardwareFlowControl = INTER_FLOW_CTRL;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                     //TX & RX enable
    USART_Init(USART1, &USART_InitStructure);

    USART_ClearFlag(USART1, USART_FLAG_TC);                                             //clear TC 
    USART_ITConfig(USART1, USART_IT_ERR, ENABLE);                                       //Error interrupt(Frame error, noise error, overrun error)

//
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
    USART_Cmd(USART1, ENABLE);
}


/**
  * @brief  usart2 rx/tx dma
  * @param  rx/tx cache
  * @param
  * @retval
  */
static void usart1_dma_init(void)
{
    DMA_InitTypeDef DMA_InitStructure;

    /* dma1 clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_Cmd(DMA1_Channel5, DISABLE);

    /* dma1 channel4/channel5 for usart1  */
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (USART1->DR);              //peripheral base address        
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                   //peripheral base address is fixed
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                            //memory address auto increment    
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;            //peripheral transfer unit is byte
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;                    //memory unit is byte
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                              //medium priority
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                       //disable memory to memory tranfers 
    DMA_InitStructure.DMA_MemoryBaseAddr = 0;
    DMA_InitStructure.DMA_BufferSize = 0;

    /* dma1 channel5 for usart1 rx */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);

    /* dma1 channel4 for usart1 tx */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);                                     //dma1 channel4 for usart1 tx     

#ifdef _DMA1_Ch5_RX_INT_
    NVIC_InitTypeDef NVIC_InitStructure;
    /* dma1 channle5 interrupt for usart1 rx TC pending */
    DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);                                  //set DMA_CCR3 of TCIE enable transfer complete interrupt
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

    /* enable dma1 4&5 to kick usart1 rx dma */
    DMA_Cmd(DMA1_Channel4, ENABLE);
    DMA_Cmd(DMA1_Channel5, ENABLE);

    /* show register */
    COMM_DBG("USART1 SR %x, CR1 %x, CR2 %x, CR3 %x\r\n", USART1->SR, USART1->CR1, USART1->CR2, USART1->CR3);
    COMM_DBG("DMA1_Channel5 of USART1_RX : ISR %x, IFCR %x, CCR %x, CNDTR %x, CPAR %x, CMAR %x\r\n", \
             DMA1->ISR, DMA1->IFCR, DMA1_Channel5->CCR, DMA1_Channel5->CNDTR, DMA1_Channel5->CPAR, DMA1_Channel5->CMAR);
    COMM_DBG("DMA1_Channel4 of USART1_TX : ISR %x, IFCR %x, CCR %x, CNDTR %x, CPAR %x, CMAR %x\r\n", \
             DMA1->ISR, DMA1->IFCR, DMA1_Channel4->CCR, DMA1_Channel4->CNDTR, DMA1_Channel4->CPAR, DMA1_Channel4->CMAR);
}


/**
  * @brief    USART1->DR DMA rx_length bytes to rx_buf by DMA1_Channel_5
  * @param    rx_buf, request dma buffer
  * @param    rx_length, request dma length
  * @param    timeout, calc second
  * @retval
  */
int usart1_rx_request(void* buffer, uint32_t length, int timeout)
{
    DMA_Cmd(DMA1_Channel5, DISABLE);                                                 //disable dma1 channel 5 to config    
    DMA_SetCurrMemoryBaseAddr(DMA1_Channel5, (uint32_t)buffer);                      //tranfer buffer
    DMA_SetCurrDataCounter(DMA1_Channel5, length & 0xffff);                          //tranfer size                                       
    DMA_Cmd(DMA1_Channel5, ENABLE);                                                  //enable dma1 channel 5                
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);                                   //kick start usart1 rx dma

	  inter_xfer_tick_1s = 0;
    while (DMA_GetFlagStatus(DMA1_FLAG_TC5) == RESET)                                //dma TC pending
    {
        if(inter_xfer_tick_1s > timeout)
        {
					  //uart rx dma timeout trigger
					  memset(buffer, DEFAULT_VALUE_FILLED, length);
            DMA_ClearFlag(DMA1_FLAG_TC5); 
					  return TRANSMISSION_TIMEOUT;
        }
    }
    DMA_ClearFlag(DMA1_FLAG_TC5);                                                     //clear

    return TRANSMISSION_OK;//(length - DMA_GetCurrDataCounter(DMA1_Channel5));
}


/**
 * @brief  tx_buf dam tx_length bytes to USART1->DR by DMA1_Channel_4
 * @param    tx_buf, request dma buffer
 * @param    tx_length,  really dma length
 * @retval
 */
int usart1_tx_request(const void* buffer, uint32_t length)
{
    DMA_Cmd(DMA1_Channel4, DISABLE);                                                    //disable dma1 channel 4 to config
    DMA_SetCurrMemoryBaseAddr(DMA1_Channel4, (uint32_t)buffer);                         //tx buffer
    DMA_SetCurrDataCounter(DMA1_Channel4, length & 0xffff);                             //tx length
    DMA_Cmd(DMA1_Channel4, ENABLE);                                                     //enable dma1 channel 4                

    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);                                      //kick start usart1 tx dma

    while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);                                  //dma TC
    DMA_ClearFlag(DMA1_FLAG_TC4);                                                       //clear

    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);

    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);                        //uart1 TC
    USART_ClearFlag(USART1, USART_FLAG_TC);                                             //clear

    //COMM_DBG("%s DMA1_Channel4 : ISR %8x, CNDTR %4x, CMAR %8x. USART1 SR %4x\r\n", __FUNCTION__, DMA1->ISR, DMA1_Channel4->CNDTR, DMA1_Channel4->CMAR, USART1->SR);

    return TRANSMISSION_OK;//(length - DMA_GetCurrDataCounter(DMA1_Channel4));
}


/**
 * @brief
 * @param
 * @retval
 */
void bsp_usart1_init(void)
{
    usart1_nvic_init();
    usart1_hw_init();
    //usart1_dma_init();
}


/**
 * @brief  DMA1 Channel5 ISR : DMA_ISR of TCIF indicates that usart1 rx data has been DMAed to buffer
 * @param
 * @retval
 */
void DMA1_Channel5_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC5) != RESET)
    {                                                   //check DMA_ISR of TCIF
        DMA_ClearITPendingBit(DMA1_IT_TC5);             //clear DMA_IFCR of CTCIF
    }
}



/**
  * @brief  USART1 rx ISR. Line Terminator : DOS Terminators - CR/LF
  * @param
  * @retval
  */
static uint8_t buf[10] = {0};
static uint8_t len = 0;
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_ORE) != RESET)
    {
        USART_ReceiveData(USART1); //clear overrun pending

        COMM_ERR("Overrun!");
    }
    else if (USART_GetITStatus(USART1, USART_IT_FE) != RESET)
    {
        USART_ReceiveData(USART1); //clear fe pending

        COMM_ERR("Framing!");
    }
    else if (USART_GetITStatus(USART1, USART_IT_NE) != RESET)
    {
        USART_ReceiveData(USART1); //clear ne pending    

        COMM_ERR("Noise!");
    }
	else if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
		buf[len++] = USART_ReceiveData(USART1);
		if(len == 10)len = 0;
	}
	/*
		MCU<-->board tong xing xie yi
	*/
}

/*********************************************END OF FILE**********************/
