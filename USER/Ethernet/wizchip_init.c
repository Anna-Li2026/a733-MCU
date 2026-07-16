
/**
* @file wizchip_cbfunc.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief
*
* @details spi callback function for accessing WIZCHIP, user should implement with your host spi peripheral
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
#include "stm32f10x.h"    
#include "stm32f10x_spi.h"
#include "trace.h"
#include "socket.h"    //just include one header for WIZCHIP
#include "dhcp.h"


/*
 * @brief
 * @details If you want to display debug & procssing message, Define _WIZCHIP_DEBUG_
 * @note    If defined, it dependens on <stdio.h>
 */

#ifdef _WIZCHIP_DEBUG_
#define WIZ_INF(fmt, ...)   INF(fmt, ##__VA_ARGS__)
#define WIZ_DBG(fmt, ...)   DBG(fmt, ##__VA_ARGS__)
#define WIZ_WRN(fmt, ...)   WRN(fmt, ##__VA_ARGS__)
#define WIZ_ERR(fmt, ...)   ERR(fmt, ##__VA_ARGS__)
#define WIZ_DUMP(buf,len)   DUMP(buf, len)
#else
#define WIZ_INF(fmt, ...)
#define WIZ_DBG(fmt, ...)
#define WIZ_WRN(fmt, ...)
#define WIZ_ERR(fmt, ...)
#define WIZ_DUMP(buf,len)
#endif


 ///////////////////////////////////
 // STM32F103X SPI PIN Definition //
 ///////////////////////////////////
#define WIZCHIP_SPI_SCS_PIN   GPIO_Pin_4
#define WIZCHIP_SPI_CLK_PIN   GPIO_Pin_5
#define WIZCHIP_SPI_MISO_PIN  GPIO_Pin_6
#define WIZCHIP_SPI_MOSI_PIN  GPIO_Pin_7
#define WIZCHIP_SPI_PORT      GPIOA

#define WIZCHIP_CHIP_RST_PIN  GPIO_Pin_5
#define WIZCHIP_CHIP_INT_PIN  GPIO_Pin_4
#define WIZCHIP_CHIP_PORT     GPIOC

///////////////////////////////////
// Default Event Flag            //
///////////////////////////////////
//typedef struct
//{
//    uint16_t con : 1;       /* 0b connection with peer */
//    uint16_t discon : 1;    /* 1b FIN/ACK packe is received from a peer */
//    uint16_t recv : 1;      /* 2b data received from a peer */
//    uint16_t timeout : 1;   /* 3b when APP/TCP timeout occurs */
//    uint16_t send_ok : 1;   /* 4b SEND command is completed */
//    uint16_t sn : 3;        /* 5~7b socket interrupted */

//    uint16_t rxdone : 1;
//    uint16_t txdone : 1;
//    uint16_t : 6;
//}EVENT_FLAGS;


//volatile EVENT_FLAGS bW5500flags;

///////////////////////////////////
// Default Network Configuration //
///////////////////////////////////


///////////////////////////////////
// Default Network Configuration //
///////////////////////////////////
wiz_NetInfo gWIZNETINFO = {
    /* default dhcp ip */
    .mac = MAC(0x00,0x50,0x56,0x7c,0x00,0x03),
    .ip = IP(0,0,0,0),
    .sn = IP(0,0,0,0),
    .gw = IP(0,0,0,0),
    .dns = IP(0,0,0,0),
    .serv = IP(192,168,1,30),
    .dhcp = NETINFO_DHCP
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Call back function for W5500 SPI - Theses used as parameter of reg_wizchip_xxx_cbfunc()  //
// Should be implemented by WIZCHIP users because host is dependent                         //
//////////////////////////////////////////////////////////////////////////////////////////////                            


/**
  * @brief  Intialize spi bus
  * @param
  * @retval
  */
static void stm32_spi_bus_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    SPI_InitTypeDef  SPIBUS1_InitStructure;

    //clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, ENABLE);

    //w5500 hardware reset pin (PC5)
    GPIO_SetBits(WIZCHIP_CHIP_PORT, WIZCHIP_CHIP_RST_PIN);    //W5500 chip hardware reset pin keep low at least 50us!!  --- Jeff.he
    GPIO_InitStructure.GPIO_Pin = WIZCHIP_CHIP_RST_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(WIZCHIP_CHIP_PORT, &GPIO_InitStructure);

    //w5500 external interrupt pin(PC4)
    GPIO_InitStructure.GPIO_Pin = WIZCHIP_CHIP_INT_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(WIZCHIP_CHIP_PORT, &GPIO_InitStructure);

    //EXTI4 Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //pc4
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4);     //Connect EXTI Line4 to PC4
    EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;            //falling trigger event
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    //w5500 chip cs pin (PA4)    
    GPIO_SetBits(WIZCHIP_SPI_PORT, WIZCHIP_SPI_SCS_PIN);
    GPIO_InitStructure.GPIO_Pin = WIZCHIP_SPI_SCS_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(WIZCHIP_SPI_PORT, &GPIO_InitStructure);

    //w5500 chip spi clock(PA5), miso(PA6), mosi(PA7)
    GPIO_SetBits(WIZCHIP_SPI_PORT, WIZCHIP_SPI_CLK_PIN | WIZCHIP_SPI_MOSI_PIN | WIZCHIP_SPI_MISO_PIN);
    GPIO_InitStructure.GPIO_Pin = WIZCHIP_SPI_CLK_PIN | WIZCHIP_SPI_MOSI_PIN | WIZCHIP_SPI_MISO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(WIZCHIP_SPI_PORT, &GPIO_InitStructure);

    /* stm32 spi init */
    SPIBUS1_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;         //full duplex
    SPIBUS1_InitStructure.SPI_Mode = SPI_Mode_Master;                              //spi master
    SPIBUS1_InitStructure.SPI_DataSize = SPI_DataSize_8b;                          //data 8b frame
    SPIBUS1_InitStructure.SPI_CPOL = SPI_CPOL_Low;                                 //spi mode0/mode3?
    SPIBUS1_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                               //fist clock rasing dege sampling
    SPIBUS1_InitStructure.SPI_NSS = SPI_NSS_Soft;                                  //NSS pin by software control
    SPIBUS1_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;         //spi bound rate
    SPIBUS1_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                         //MSB trasfer first
    SPIBUS1_InitStructure.SPI_CRCPolynomial = 7;                                   //crc 7
    SPI_Init(SPI1, &SPIBUS1_InitStructure);

    SPI_Cmd(SPI1, ENABLE);                                                         //enable stm32 spi bus1
}


/**
 * @brief
 * @param
 * @retval
 */
__attribute__((unused)) static void spi_dma_init(void)
{
    DMA_InitTypeDef DMA_InitStructure;

    /* dma1 clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA1_Channel2);
    DMA_DeInit(DMA1_Channel3);

    /* dma1 channel2/channel2 for spi1  */
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (SPI1->DR);              //peripheral base address        
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                 //peripheral base address is fixed
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                          //memory address auto increment    
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;          //peripheral transfer unit is byte
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;                  //memory unit is byte
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                            //medium priority
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                     //disable memory to memory tranfers 
    DMA_InitStructure.DMA_MemoryBaseAddr = 0;
    DMA_InitStructure.DMA_BufferSize = 0;

    /* dma1 channel2 for spi1 rx */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);

    /* dma1 channel3 for spi1 tx */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);

    DMA_ITConfig(DMA1_Channel2, DMA_IT_TC | DMA_IT_HT | DMA_IT_TE, DISABLE);
    DMA_ITConfig(DMA1_Channel3, DMA_IT_TC | DMA_IT_HT | DMA_IT_TE, DISABLE);

    /* enable dma1 2&3 */
    DMA_Cmd(DMA1_Channel2, ENABLE);
    DMA_Cmd(DMA1_Channel3, ENABLE);

    /* show register */
    //WIZ_DBG("\r\nDMA1_Channel2 of SPI1_RX : ISR %x, IFCR %x, CCR %x, CNDTR %x, CPAR %x, CMAR %x\r\n", \
    //DMA1->ISR, DMA1->IFCR, DMA1_Channel2->CCR, DMA1_Channel2->CNDTR, DMA1_Channel2->CPAR, DMA1_Channel2->CMAR);
    //WIZ_DBG("DMA1_Channel3 of SPI1_TX : ISR %x, IFCR %x, CCR %x, CNDTR %x, CPAR %x, CMAR %x\r\n", \
    //DMA1->ISR, DMA1->IFCR, DMA1_Channel3->CCR, DMA1_Channel3->CNDTR, DMA1_Channel3->CPAR, DMA1_Channel3->CMAR);

}


/**
 * @brief
 * @param
 * @retval
 */
__attribute__((unused)) static void spi_rx_request(const void* cache, uint32_t length)
{
    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA_SetCurrMemoryBaseAddr(DMA1_Channel2, (uint32_t)cache);
    DMA_SetCurrDataCounter(DMA1_Channel2, length);

    //WIZ_DBG("\r\n1 %s DMA1_Channel2 : ISR %8x, CNDTR %4x, CMAR %8x, length %x. SPI1 SR %4x, CR1 %04x\r\n", \
    //        __FUNCTION__, DMA1->ISR, DMA1_Channel2->CNDTR, DMA1_Channel2->CMAR, length, SPI1->SR, SPI1->CR1);    

    DMA_Cmd(DMA1_Channel2, ENABLE);
    SPI_DirectionalModeConfig(SPI1, SPI_Direction_2Lines_RxOnly);                    //config read only at DMA rx request


    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);                                //kick start spi1 rx dma    

    while (DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET);                                //dma TC
    DMA_ClearFlag(DMA1_FLAG_GL2);                                                    //clear

    SPI_DirectionalModeConfig(SPI1, SPI_Direction_2Lines_FullDuplex);                //restored full duplex mode

    //WIZ_DBG("2 %s DMA1_Channel2 : ISR %8x, CNDTR %4x, CMAR %8x, length %x. SPI1 SR %4x, CR1 %04x\r\n", \
    //        __FUNCTION__, DMA1->ISR, DMA1_Channel2->CNDTR, DMA1_Channel2->CMAR, length, SPI1->SR, SPI1->CR1);
}


/**
 * @brief
 * @param
 * @retval
 */
__attribute__((unused)) static void spi_tx_request(const void* cache, uint32_t length)
{
    DMA_Cmd(DMA1_Channel3, DISABLE);                                                //disable dma1 channel 3 to config    
    DMA_SetCurrMemoryBaseAddr(DMA1_Channel3, (uint32_t)cache);
    DMA_SetCurrDataCounter(DMA1_Channel3, length);
    DMA_Cmd(DMA1_Channel3, ENABLE);                                                //enable dma1 channel 3                
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);                                //kick start spi1 tx dma

    while (DMA_GetFlagStatus(DMA1_FLAG_TC3) == RESET);                                //dma TC
    DMA_ClearFlag(DMA1_FLAG_GL3);                                                    //clear

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != SET);                    //wait TXE == 1

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) != RESET);                    //BSY == 0

    //WIZ_DBG("%s DMA1_Channel3 : ISR %8x, CNDTR %4x, CMAR %8x, length %x. SPI1 SR %4x\r\n", \
    //        __FUNCTION__, DMA1->ISR, DMA1_Channel3->CNDTR, DMA1_Channel3->CMAR, length, SPI1->SR);
}



/**
 * @brief  EXTI4 ISR
 * @param
 * @retval
 */
void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line4);

        /* clear IR and Sn_IR pending to pull up the INTn pin */
        /* setIR(getIR())             :    if IR isn't equal to '0', INTn PIN is asserted low until it is '0' */
        /* setSn_IR(n, getSn_IR(n)) :    if Sn_IR is not equal to '0', the n-th bit of SIR is '1' and INTn PIN is asserted until SIR is '0' */

        //printf("INTn IR %2x, SIR %2x\n", getIR(), getSIR());
    }
}


/**
 * @brief
 * @param
 * @retval
 */
__attribute__((unused)) static void SPI_CrisEnter(void)
{
    __set_PRIMASK(1);
}


/**
 * @brief
 * @param
 * @retval
 */
__attribute__((unused)) static void SPI_CrisExit(void)
{
    __set_PRIMASK(0);
}

/**
 * @brief  w5500 hardware reset
 * @param
 * @retval
 */
static void wizchip_hw_reset(void)
{
    GPIO_ResetBits(WIZCHIP_CHIP_PORT, WIZCHIP_CHIP_RST_PIN);        //ww5500 chip reset pin low
    delay_us(600);                                                  //Trc : Reset Cycle Time Min 500us
    GPIO_SetBits(WIZCHIP_CHIP_PORT, WIZCHIP_CHIP_RST_PIN);          //w5500 chip reset pin high
    delay_ms(1600);                                                 //Tpl : RSTn to internal PLOCK(PLL Lock) Max 2ms, but here waiting for 1 second                                
}

/**
 * @brief  w5500 chip cs low
 * @param
 * @retval
 */
static void  wizchip_select(void)
{
    GPIO_ResetBits(WIZCHIP_SPI_PORT, WIZCHIP_SPI_SCS_PIN);
}

/**
 * @brief  w5500 chip cs high
 * @param
 * @retval
 */
static void  wizchip_deselect(void)
{
    GPIO_SetBits(WIZCHIP_SPI_PORT, WIZCHIP_SPI_SCS_PIN);
}


/**
 * @brief  spi full duplex mode, sends a byte through the SPI interface and return the byte received from the SPI bus.
 * @param  wb, byte to write
 * @retval byte received
 */
static uint8_t spi_full_duplex(uint8_t wb)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);  //loop while DR register in not empty
    SPI_I2S_SendData(SPI1, wb);                                        //send byte through the SPI1 peripheral    
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET); //waiting a byte received by pending
    return SPI_I2S_ReceiveData(SPI1);                                //return the byte from the spi bus
}


/**
 * @brief  stm32 write wizchip one byte by spi bus, elapsed 1.63us per write one byte
 * @param wb, write one byte
 * @retval
 */
__attribute__((always_inline)) static void  wizchip_write(uint8_t wb)
{
    //SPI_I2S_SendData(SPI1, wb);                                        //send byte through the SPI1 peripheral
    //while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);      //loop while DR register in not empty
    spi_full_duplex(wb);
}

/**
 * @brief  stm32 read wizchip one byte by spi bus, elapsed 2.02us per read one byte
 * @param
 * @retval read one byte
 */
__attribute__((always_inline)) static uint8_t wizchip_read(void)
{
    //SPI_I2S_ReceiveData(SPI1);
    //SPI_I2S_SendData(SPI1, 0);                                        //send byte through the SPI1 peripheral
    //while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);      //loop while DR register in not empty
    //return SPI_I2S_ReceiveData(SPI1);
    return spi_full_duplex(0);                                            //0 dummy byte
}

//////////////////////////////////////////////////////////////////////////
void show_network(void)
{
		uint8_t tmpstr[6];

    /* get info */
    ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);
    // Display Network Information
    ctlwizchip(CW_GET_ID, (void*)tmpstr);
    WIZ_INF("\r\n=== %s NET CONF ===\r\n", (char*)tmpstr);
    WIZ_INF("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", gWIZNETINFO.mac[0], gWIZNETINFO.mac[1], gWIZNETINFO.mac[2], gWIZNETINFO.mac[3], gWIZNETINFO.mac[4], gWIZNETINFO.mac[5]);
    WIZ_INF("SIP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]);
    WIZ_INF("GAR: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0], gWIZNETINFO.gw[1], gWIZNETINFO.gw[2], gWIZNETINFO.gw[3]);
    WIZ_INF("SUB: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0], gWIZNETINFO.sn[1], gWIZNETINFO.sn[2], gWIZNETINFO.sn[3]);
    WIZ_INF("DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0], gWIZNETINFO.dns[1], gWIZNETINFO.dns[2], gWIZNETINFO.dns[3]);
    WIZ_INF("======================\r\n");
}


/**
 * @brief  login the WIZCHIP
 * @param
 * @retval
 */
extern uint32_t __Gen_MAC;
void wizchip_login(void)
{
    uint8_t tmp;
    uint8_t memsize[2][8] = {0};
	
		/* get mac from extern tool */
		if(*((const uint64_t *)&__Gen_MAC) != 0)
			memcpy(gWIZNETINFO.mac , (uint8_t*)&__Gen_MAC, 6);
 
    /* init rx/tx socket0~7 buffer */
    //for (tmp = 0; tmp < 2 * _WIZCHIP_SOCK_NUM_; ++tmp)
    //    memsize[tmp / _WIZCHIP_SOCK_NUM_][tmp & 7] = SOCKET_CACHE_KB;
		memsize[0][0] = SOCKET_CACHE_KB;
		memsize[1][0] = SOCKET_CACHE_KB;

    stm32_spi_bus_init();
#if (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_DMA_)
    spi_dma_init();
#endif

    //first of all, should register spi callback functions implemented by suer for accessing WIZCHIP
#if (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_) || (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_DMA_)
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
#elif _WIZCHIP_IO_MODE_ == _WIZCHIP_TO_MODE_SPI_FDM_
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_select);    //CS must be tried with LOW
#else
#if (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_) != _WIZCHIP_IO_MODE_SPI_
#error "Unknown _WIZCHIP_IO_MODE_"
#else
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
#endif
#endif

    //WIZCHIP critical section
    //reg_wizchip_cris_cbfunc();

    //SPI Read & Write callback function
    reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);

    //SPI DMA callback function
    reg_wizchip_spi_dma_cbfunc(spi_tx_request, spi_rx_request);

    //hardreset, hw reset before using w5500
    wizchip_hw_reset();

    //wizchip initialize
    if (ctlwizchip(CW_INIT_WIZCHIP, (void*)memsize) == -1)
    {
        WIZ_ERR("WIZCHIP initialized fail.\r\n");
    }

    //MAC, Gateway IP, Subnet Mask IP, Source IP initialize
    if (ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO))
    {
        WIZ_ERR("MAC initialized fail.\r\n");
    }

    //wizchip interrupt initialize.When an interrupt is generated, the INTn pin is pulled low!
    //IR     :    interrupt types(IP conflict, destination unreachable, PPPoE close). If IR is not 0, INTn pin is pulled low until 0x00.
    //IMR    :    mask IR interrupt.
    //SIR    :    interrupt(socket 0~7). If Sn_IR is not equal to 0x00 the n-th bit of SIR is and INTn pin is asserted until SIR is 0x00.
    //SIMR    :    mask SIR interrupt.

    //Sn_IR :    socket interrupt types(SEND_OK, TIMEOUT, RECV, DISCON, CON).
    //Sn_IMR:    interrupt mask Sn_IR.
    //Sn_SR    :    socket status(CLOSED, INIT, LISTEN, ESTABLISHED, CLOSE_WAIT, UDP, MACRAW).

    uint16_t chip_imr;
    uint8_t  sock_imr, sock_io;

    /* socket0, ip conflict, dest unreach interrupt */
    //chip_imr = IK_SOCK_7 | IK_SOCK_6 | IK_SOCK_0 | IK_IP_CONFLICT | IK_DEST_UNREACH;
		chip_imr = IK_SOCK_0 | IK_IP_CONFLICT | IK_DEST_UNREACH;
    ctlwizchip(CW_SET_INTRMASK, &chip_imr);

    /* socket n interrupt */
    sock_imr =  SIK_SENT | SIK_TIMEOUT | SIK_RECEIVED | SIK_DISCONNECTED | SIK_CONNECTED;
    ctlsocket(0, CS_SET_INTMASK, &sock_imr);
		sock_io = SOCK_IO_BLOCK;
		ctlsocket(0, CS_SET_IOMODE, &sock_io);
    //ctlsocket(6, CS_SET_INTMASK, &sock_imr);
    //ctlsocket(7, CS_SET_INTMASK, &sock_imr);

    WIZ_INF("\nw5500 IR %x, IMR %x, SIR %x, SIMR %x\n", getIR(), getIMR(), getSIR(), getSIMR());
    for (tmp = 0; tmp < 8; ++tmp)
        WIZ_INF("socket%d IR %x, IMR %x, rx %dKB, tx %dKB\n", tmp, getSn_IR(tmp), getSn_IMR(tmp), getSn_RXBUF_SIZE(tmp), getSn_TXBUF_SIZE(tmp));

    //PHY link status check
    do
    {
        if (ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1)
        {
            WIZ_WRN("Unknown PHY link status.\n");
        }

        WIZ_DBG("LINK %02x\b\b\b\b\b\b\b", tmp);
    } while (tmp == PHY_LINK_OFF);

}

