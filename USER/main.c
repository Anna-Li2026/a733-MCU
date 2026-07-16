
/**
* @file main.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief main
*
* @details W5500 works in server mode, waiting for the pc client to actively connect.
*           After the connection is successful, processing the transaction
*
*           STM32 - W5500
*           PC5 - W5500_RST
*           PC4 - W5500_INT
*           PA4 - W5500_SCS
*           PA5 - W5500_SCK
*           PA6 - W5500_MISO
*           PA7 - W5500_MOSI
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
#include "../version"
#include <string.h>
#include <stdint.h>
#include "stm32f10x.h"
#include "SysTick.h"
#include "trace.h"
#include "bsp_timer2.h"
#include "bsp_uart4.h"
#include "bsp_inter_xfer.h"
#include "bsp_usart3.h"
#include "bsp_eeprom.h"
#include "nr_micro_shell.h"
#include "socket.h"             //just include one header for WIZChip
#include "dhcp.h"
#include "protocol.h"
#include "bsp_burnin_board.h"
#include "burnin_board_map.h"
#include "tcp_client.h"

////////////////////////////////////////////////
// Shared Buffer Definition for project       //
////////////////////////////////////////////////
//head size sizeof(ITVL_t)
//content size 32KB
//end '\r\n\0'
uint8_t gDATABUF[8192 - MAX_TRANSMISSION_LENGTH] __attribute__((aligned(8)));
ITVL_t* gITVL = (ITVL_t*)gDATABUF;
ITVL_t itvl;
ITVL_t itvl_recv;
ITVL_t itvl_tim;
uint8_t itvl_init_flag=0;
//volatile uint32_t sec = 0;

/////////////////////////////////////////
// SOCKET NUMBER DEFINION for Examples //
/////////////////////////////////////////
#define SOCK_TCPS        0
#define SOCK_UDPS        6
#define SOCK_TCPC        0

/******IP******/
//uint8_t  remote_ip[4]={169,254,245,35};		//填入PC（服务器）端的IP号
//uint16_t remote_port=8004;					//填入PC（服务器）端的端口号
//uint8_t local_ip[4]  ={169,254,245,100};
//uint8_t subnet[4]    ={255,255,0,0};
//uint8_t gateway[4]   ={169,254,1,1};
//uint8_t dns_server[4]={114,114,114,114};
//uint16_t local_port=8000;
uint8_t xh=0;

uint8_t  remote_ip[4]={192,168,12,110};		//填入PC（服务器）端的IP号
uint16_t remote_port=8000;					//填入PC（服务器）端的端口号
uint8_t local_ip[4]  ={192,168,12,101};
uint8_t subnet[4]    ={255,255,255,0};
uint8_t gateway[4]   ={192,168,12,1};
uint8_t dns_server[4]={114,114,114,114};
uint16_t local_port=8000;

#if 0
#define BUFF_SIZE   2048
uint8_t downlink[BUFF_SIZE] __attribute__((aligned(8)));
uint8_t uplink[BUFF_SIZE]   __attribute__((aligned(8)));
#endif

#if 0
/**
 * @brief stm32 system PLL 72MHZ
 *
 */
static void RCC_Configuration(void)
{
    ErrorStatus HSEStartUpStatus;                              //HSE work status

    RCC_DeInit();                                              //all registers about clock set default
    RCC_HSEConfig(RCC_HSE_ON);                                 //HSE switch on
    HSEStartUpStatus = RCC_WaitForHSEStartUp();                //wait for HSE to stabilize

    if (SUCCESS == HSEStartUpStatus)                            //stabilized
    {
        /* Enable Prefetch Buffer */
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);    //set FLASH
        /* Flash 2 wait state */
        FLASH_SetLatency(FLASH_Latency_2);


        RCC_HCLKConfig(RCC_SYSCLK_Div1);                         //AHB 72/1MHZ
        RCC_PCLK2Config(RCC_HCLK_Div1);                          //APB2 and HCLK 72MHZ
        RCC_PCLK1Config(RCC_HCLK_Div2);                          //APB1 72/2MHz

#ifndef STM32F10X_CL
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);     //PLLCLK = 8MHz * 9 = 72 MHz
#else
        /* MCU are STM32F107x or STM32F105x */
        /* PLL2 configuration: PLL2CLK = (HSE / 5) * 8 = 40 MHz */
        RCC_PREDIV2Config(RCC_PREDIV2_Div5);
        RCC_PLL2Config(RCC_PLL2Mul_8);

        RCC_PLL2Cmd(ENABLE);                                     //enable PLL2
        while (RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET);    //wait for stabilize

        /* PLL configuration: PLLCLK = (PLL2 / 5) * 9 = 72 MHz */
        RCC_PREDIV1Config(RCC_PREDIV1_Source_PLL2, RCC_PREDIV1_Div5);
        RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_9);
#endif

        RCC_PLLCmd(ENABLE);                                         //enable PLL
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);        //wait for stabilize

        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);                  //system clock source is PLL

        while (RCC_GetSYSCLKSource() != 0x08);                      //check clock source
        RCC_ClockSecuritySystemCmd(ENABLE);                         //enable clock security

        /* Enable peripheral clocks --------------------------------------------------*/
        /* Enable I2C1 and I2C1 clock */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

        /* Enable GPIOA GPIOB SPI1 and USART1 clocks */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
                               | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD
                               | RCC_APB2Periph_AFIO, ENABLE);
    }
}
#endif

/**
 * @brief stm32 interrupt vector table configuration
 */
static void NVIC_Configuration(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//Set 2bit preempt priority and 2bit subpriority group

/*  preempt    subpriority    interrupt
 *      2        3            W5500 EXIT
 *      3        0            USART2
 *      3        1            USART3
 *      3        2            USART1
 *      3        3            TIMER2
 */

    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);    //Set the Vector Table base location at 0x08000000
}



/**
 * @brief platform (STM32F103X) initialization for peripherals as GPIO, SPI, UARTs
 */
static void System_Initialization(void)
{
    RCC_ClocksTypeDef  rcc_clocks;
    //RCC_Configuration();                                  //System clock 72MHZ

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    RCC_GetClocksFreq(&rcc_clocks);
    NVIC_Configuration();                                   //STM32 interrupt vector

    SysTick_Init(rcc_clocks.SYSCLK_Frequency/1000000);      //System Tick

    TRIGGER_INIT();

    UART4_Init(115200);

    INF("Version(%s-%s %s). SCLK %dMHz, HCLK %dMHz, PCLK1 %dMHz, PCLK2 %dMHz\n",\
        __TAG__, __COMMIT__, __BUILT__, \
        rcc_clocks.SYSCLK_Frequency/1000000,\
        rcc_clocks.HCLK_Frequency/1000000,\
        rcc_clocks.PCLK1_Frequency/1000000,\
        rcc_clocks.PCLK2_Frequency/1000000);

#ifdef _SIMULATION_MTK_SOC_
    #warning "usart3 2250Kbps"
        bsp_usart3_init(INTER_COMM_BOUND);                      //USART3 fisrt init loopback between usart2 and usart3
#else
    Timer2_Init_Config();                                   //Timer2 init
    burnin_init();                                          //burnin board init
#endif

}
#if 0
/**
  * @brief  tcps using ioLibrary_BSD
  * @param
  * @retval
  */
int32_t tcps_handler(uint8_t sn, uint16_t port)
{
    //int8_t err;
    //uint8_t sn_sr;
    int32_t ret;
    uint16_t size = 0;
    uint16_t sentsize = 0;

    //if ((err = getsockopt(sn, SO_STATUS, &sn_sr)) != SOCK_OK)
    //{
    //  WRN("SO_STATUS fail %x\n", err);
    //     return -1;
    //}
    //INF("%2x %2x %2x\b\b\b\b\b\b\b\b", sn_sr, getSn_SR(sn), getSn_IR(sn));

    /* checking whether the command is completed or not, please check the Sn_SR or Sn_IR */
    switch (getSn_SR(sn))
    {
    case SOCK_ESTABLISHED:
        /**  0x17
          *  This indicates the status of the connection of socket n
          *  It changes to SOCK_ESTABLISHED :
          *  1.When the 'TCP SERVER' processed the SYN packet from the 'TCP CLIENT' during SOCK_LISTEN
          *  2.When the CONNECT command is successful
          *  During SOCK_ESTABLISHED, DATA packet can be transferred using SEND or RECV command.
          *
          *  Sn_IR(Socket interrupt register checking) :
          *    Sn_IR_SENDOK(1<<4)     : SEND OK interrupt
          *    Sn_IR_TIMEOUT(1<<3) : TIMEOUT interrupt
          *    Sn_IR_RECV(1<<2)     : RECV interrupt
          *     Sn_IR_DISCON(1<<1)     : DISCON interrupt
          *     Sn_IR_CON(1<<0)     : CON interrupt
          *
          */

        if (getSn_IR(sn) & Sn_IR_CON)
        {
            INF("%d:Connected\n", sn);
            /* clear Sn_IR_CON pending */
            setSn_IR(sn, Sn_IR_CON);
        }

        if (uplink_flag)
        {
            /**    socket n TX buffer :
              *   _____max
              *       | |
              *    | |
              *    | Sn_TX_FSR[R](the free size of TX buffer)
              *    | |
              *       |_|____________Sn_TX_WR
              *       |            |
              *    |         |
              *    SnTX_BUF     | SEND transmits the saved data from Sn_TX_RD to Sn_TX_WR
              *       |         |
              *       |_________|____Sn_TX_RD[R]
              *   _|_0
              */

            TRIG_LOW(TRIG1);
            size = PACKET_SIZE;
            sentsize = 0;
            while (size != sentsize)
            {
                ret = send(sn, uplink + sentsize, size - sentsize);
                if (ret < 0)
                {
                    close(sn);
                    return ret;
                }
                sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
            }
            uplink_flag = 0;
            TRIG_HIGH(TRIG1);
            //printf("S%x_IR %2x, S%x_TX_FSR %4x, S%x_TX_RD %4x, S%x_TX_WR %4x\r\n", sn,getSn_IR(sn), sn,getSn_TX_FSR(sn), sn,getSn_TX_RD(sn), sn,getSn_TX_WR(sn));
        }


        /* data come from w5500 */
        if (getSn_IR(sn) & Sn_IR_RECV/*(size = getSn_RX_RSR(sn)) > 0*/)
        {
            /**    socket n RX buffer :
              *   ___max
              *       |
              *       |______________Sn_RX_WR[R]
              *       |            |
              *    Sn_RX_BUF    |
              *       |     Sn_RX_RSR[R](the data size received and saved in RX buffer)
              *       |         |
              *       |_________|____Sn_RX_RD
              *   _|_0
              */

            size = getSn_RX_RSR(sn);
            if (size > BUFF_SIZE)
                size = BUFF_SIZE;

            TRIG_LOW(TRIG0);
            ret = recv(sn, downlink, size);
            if (ret <= 0)
                return ret;
            TRIG_HIGH(TRIG0);

            /* a packet of 512B elapsed: 3.2us */
            TRIG_LOW(TRIG0);
            usart2_tx_request(size);
            TRIG_HIGH(TRIG0);

            /* clear RECV pending to rise INTn up */
            setSn_IR(sn, Sn_IR_RECV);
        }

        break;

    case SOCK_CLOSE_WAIT:
        /**  0x1c
          *  This indicates that socket n received the disconnect-request(FIN package) from the connected peer.
          *  This is half-closing status and data can be transferred. For full-closing, DISCON command is used.
          *  But for just-closing, CLOSE command is used.
          */

        INF("%d:CloseWait\n", sn);
        if ((ret = disconnect(sn)) != SOCK_OK)
            return ret;
        INF("%d:Closed\n", sn);
        break;

    case SOCK_INIT:
        /**  0x13
          *  This indicates that socket n is opened with TCP mode.
          *  It changes to SOCK_INIT :
          *  When Sn_MR(P[3:0])='0001' and OPEN command is ordered
          *  After SOCK_INIT, user can use LISTEN or CONNECT command
          */
#ifdef _TCP_CLIENT_
        if ((ret = connect(sn, gWIZNETINFO.serv, port)) < 0)
            return ret;
        INF("%d:Connecting ip & port %d\n", sn, ret);
#else
        INF("%d:Listen, port [%d]\n", sn, port);
        if ((ret = listen(sn)) != SOCK_OK)
            return ret;
#endif
        break;

    case SOCK_CLOSED:
        /**  0x00
          *  This indicates that socket n is released.
          *  It changes to SOCK_CLOSED :
          *  1.When DICON, CLOSE command is ordered
          *  2.When a timeout occurs
          */

        INF("%d:LBTStart\n", sn);
        if ((ret = socket(sn, Sn_MR_TCP, port, 0x00)) != sn)
            return ret;
        break;

    case SOCK_LISTEN:
        /**  0x14
          *  This indicates that socket n is operating as 'TCP SERVER' mode and waiting for connection-request(SYN packet) from a peer('TCP CLIENT')
          */

          /**** temporary status ****/
    case SOCK_SYNSENT:
        /**  0x15
          *  This indicates that socket n sent the connect-request packet(SYN packet) to a peer.
          *  It is changed from SOCK_INIT to SOCK_ESTABLISHED by CONNECT command.
          *
          */

    case SOCK_SYNRECV:
        /**  0x16
          *  This indicates that socket n successfully received the connect-request packet(SYN packet) from a peer.
          *
          */

    case SOCK_FIN_WAIT:
    case SOCK_CLOSING:
    case SOCK_TIME_WAIT:
        /**  0x18 0x1a 0x1b
          *  This indicates that socket n is closing.
          *
          */

    case SOCK_LAST_ACK:
        /**  0x1d
          *  This indicates that socket n is waiting for response(FIN/ACK packet) to the disconnect-request(FIN package) by passive-close.
          *  It changes to SOCK_CLOSED when socket n received the response successfully, or when timeout occurs(Sn_IR[TIMEOUT]='1')
          */

    default:
        break;
    }

    return 1;
}
#endif

#if 0
/**
 * @brief  udps handler using ioLibrary_BSD
 * @param
 * @retval
 */
int32_t udps_handler(uint8_t sn, uint16_t port)
{
    int32_t  ret;
    uint16_t size, sentsize;
    uint8_t  destip[4];
    uint16_t destport;

    switch (getSn_SR(sn))
    {

    case SOCK_UDP:
        /**  0x22
          *  This indicates that socket n is opened in UDP mode(Sn_MR(P[3:0])='0010').
          *  It changes to SOCK_UDP :
          *  1.When Sn_MR('0010') and OPEN command is ordered
          *  Unlike TCP mode, data can be transfered without the connection-process.
          */

        if ((size = getSn_RX_RSR(sn)) > 0)
        {

            if (size > BUFF_SIZE) size = BUFF_SIZE;
            ret = recvfrom(sn, downlink, size, destip, (uint16_t*)&destport);
            if (ret <= 0)
            {
                ERR("%d: recvfrom error. %d\n", sn, ret);
                return ret;
            }

            /* loopback */
            size = (uint16_t)ret;
            sentsize = 0;
            while (sentsize != size)
            {
                ret = sendto(sn, downlink + sentsize, size - sentsize, destip, destport);
                if (ret < 0)
                {
                    ERR("%d: sendto error. %d\n", sn, ret);
                    return ret;
                }
                sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
            }
        }
        break;

    case SOCK_CLOSED:
        /**  0x00
          *  This indicates that socket n is released.
          *  It changes to SOCK_CLOSED :
          *  1.When DICON, CLOSE command is ordered
          *  2.When a timeout occurs
          */
        INF("%d:LBUStart\n", sn);
        if ((ret = socket(sn, Sn_MR_UDP, port, 0x00)) != sn)
            return ret;
        INF("%d:Opened, port [%d]\n", sn, port);
        break;

    case SOCK_MACRAW:
        /**  0x42
          *  This indicates that socket 0 is opened in MACRAW mode(Sn_MR(P[3:0])='0100').
          *  It changes to SOCK_MACRAW :
          *  1.When S0_MR('0100') and OPEN command is ordered
          *  Like UDP mode socket, MACRAW mode socket0 can transfer a MAC packet(Ethernet frame) without the connection-process.
          */
        break;

    default:
        break;
    }
    return 1;
}
#endif
//////////////////////////////////


// /**
//  * @brief    get a cbw packet from server
//  * @param
//  * @retval really length received
//  */
//static int32_t getCBW(uint8_t sn, CBW_t* cbw)
//{
//    int32_t ret;

//    if (sizeof(CBW_t) != getSn_RX_RSR(sn))
//    {
//        return CBW_PACKET_SIZE_ERR;
//    }

//    ret = recv(sn, (uint8_t*)cbw, sizeof(CBW_t));
//    if (ret < 0)
//        close(sn);

////    DBG("ret %d\n" \
////        "BIB #%d\n" \
////        "SITE #%d\n" \
////        "dCBWSignature %x\n" \
////        "dCBWTag %x\n" \
////        "dCBWDataTransferLength %x\n" \
////        "bmCBWFlags %x\n" \
////        "bCBWLUN %x\n" \
////        "bCDBLength %x\n" \
////        "CDB.opera %x\n", \
////        ret, \
////        cbw->CDB.transfer.bib, \
////        cbw->CDB.transfer.site, \
////        cbw->dCBWSignature, \
////        cbw->dCBWTag, \
////        cbw->dCBWDataTransferLength, \
////        cbw->bmCBWFlags, \
////        cbw->bCBWLUN, \
////        cbw->bCDBLength, \
////        cbw->CDB.opera);

//    return ret;
//}



///**
// * @brief    receive data from server
// * @param
// * @retval really length received
// */
//static int32_t recvData(uint8_t sn, uint8_t* data, uint32_t length)
//{
//    int32_t ret, received;
//    uint16_t size;

//    for (received = 0; (length - received) > 0; received += ret)
//    {

//        while ((size = getSn_RX_RSR(sn)) == 0);  //waitting for data forver

//        ret = recv(sn, data + received, size);
//        if (ret < 0)
//        {
//            close(sn);
//            return ret;
//        }
//    }

//    return received;
//}


///**
// * @brief    send data to server
// * @param
// * @retval really length sent
// */
//static int32_t sendData(uint8_t sn, const uint8_t* data, uint32_t length)
//{
//    int32_t ret, residue;

//    for (residue = length; residue > 0; data += ret, residue -= ret)
//    {
//				do{
//					ret = send(sn, (uint8_t*)data, (residue > SOCKET_CACHE_LENGTH) ? SOCKET_CACHE_LENGTH : residue);
//					if (ret < 0)
//					{
//							close(sn);
//							return ret;
//					}
//				}while(ret == SOCK_BUSY);
//    }

//    return (length - residue);
//}


///**
// * @brief    send a csw packet to server
// * @param
// * @retval really length sent
// */
//static int32_t sendCSW(uint8_t sn, CSW_t* csw, uint32_t tag, uint32_t residue, uint8_t status)
//{
//    int32_t ret;

//    csw->dCSWSignature = CSW_SIGNATURE;
//    csw->dCSWTag = tag;
//    csw->dCSWDataResidue = residue;
//    csw->bCSWStatus = status;
//
//	  do{
//			ret = send(sn, (uint8_t*)csw, sizeof(CSW_t));
//			if(ret < 0)
//			{
//				 close(sn);
//				 return ret;
//			}
//		}while(ret == SOCK_BUSY);

////    DBG("ret %d\n"
////        "dCSWSignature %x\n" \
////        "dCSWTag %x\n" \
////        "dCSWDataResidue %x\n" \
////        "bCSWStatus %x\n", \
////        ret, \
////        csw->dCSWSignature, \
////        csw->dCSWTag, \
////        csw->dCSWDataResidue, \
////        csw->bCSWStatus);
//
//    return ret;
//}



/**
 * @brief
 * @param
 * @retval
 */
//static int32_t inquire(int checkbib, uint32_t len, const uint16_t* serial, void* buff)
//{
//    int bib, site, str_len;
//    uint8_t str[sizeof(ITVL_t) + 64];
//    BIB_INFO_t *info;
//    ITVL_t  *pkg = (ITVL_t*)str;
//
//    memset(buff, 0 , len);
//    for (bib = 0; bib < TL_BURNIN_BOARDS; ++bib)
//    {
//       info = (BIB_INFO_t*)((uint8_t*)buff + bib*(sizeof(BIB_INFO_t) + sizeof(SLT_INFO_t)*BIB_SKTS));
//       if(check_online(bib, checkbib))
//       {
//				    //hot plug biboard
//						info->serial = biboard_init(serial[bib]);
//            info->sites  = BIB_SKTS;
//            info->size   = sizeof(BIB_INFO_t) + sizeof(SLT_INFO_t)*BIB_SKTS;
//						snprintf(info->st_version, sizeof(info->st_version), "%s-%s", __TAG__, __COMMIT__);
//
//						for(site=0; site<BIB_SKTS; ++site)
//						{
//								if(select(bib, site))
//								{
//										//elapsed 5.76ms/site
//										//TRIG_LOW();
//										str_len = snprintf((char*)pkg->value, SHELL_STR_WRAPPER_LENGTH, XFER_SHELL_CMD_TEMPLATE(check_soc, SHELL_XFER_SICO, %d, %d, %d), bib, site, sizeof(SLT_INFO_t));
//										inter_boards_txd(pkg, str_len, ITVL_ASCII_TYPE);
//										inter_boards_rxd(pkg, sizeof(SLT_INFO_t), ITVL_HEX_TYPE, 1);
//									  memcpy(&(info->slt[site]), pkg->value, sizeof(SLT_INFO_t));
//										//TRIG_HIGH();
//
//										if(pkg->value[0] != site)
//										{
//												snprintf((char*)str, sizeof(str), "#%d %d offline to dump", bib, site);
//												DUMP((char*)str, pkg->value, sizeof(SLT_INFO_t));
//										}
//								}
//								unselect(bib, site);
//						}
//       }
//			 else
//			 {
//					  memset(info, 0, sizeof(BIB_INFO_t));
//				    memset(info->slt, DEFAULT_VALUE_FILLED, sizeof(SLT_INFO_t)*BIB_SKTS);
//						WRN("slot#%d offline\n", bib);
//			 }
//    }
//
//    return TRANSMISSION_STATUS_OK;
//}


/**
 * @brief
 * @param
 * @retval 0 is ok; !0 isn't ok.
 */
// static int32_t read(int bib, int site, int medium, int partition, uint32_t lba, uint32_t len, ITVL_t* pkg)
// {
//    int32_t str_len, ret = TRANSMISSION_OK;
//
//    switch (medium)
//    {
//        case MEDIMU_EMMC:
//            if(select(bib, site))
//            {
//                /* make to tx shell command "access_blkdev sico <partition> <lba> <len>" */
//                str_len = snprintf((char*)pkg->value, SHELL_STR_WRAPPER_LENGTH, XFER_SHELL_CMD_TEMPLATE(access_blkdev, SHELL_XFER_SICO, %d, %d, %d), partition, lba, len);
//                inter_boards_txd(pkg, str_len, ITVL_ASCII_TYPE);
//                /* sico <data> */
//                ret = inter_boards_rxd(pkg, len, ITVL_HEX_TYPE, INTER_COMM_TIMEOUT_SECOND);
//                /* here no pending */
//            }
//            break;
//
//        case MEDIMU_E2PROM:
//            if(select(bib, site))
//            {
//                I2C_EE_SetDevAddr(SLT_EEPROM_ADDRESS);
//                I2C_EE_BufferRead(pkg->value, lba, len);
//            }
//            break;
//
//        case MEDIMU_NAND:
//        case MEDIMU_NOR:
//        case MEDIMU_LPDDR:
//        default:break;
//    }
//
// 		unselect(bib, site);
//    return ret;
// }


/**
 * @brief
 * @param
 * @retval
 */
// static int32_t write(int bib, int site, int medium, int partition, uint32_t lba, uint32_t len, ITVL_t* pkg)
// {
//    int str_len, ret = TRANSMISSION_OK;
//    uint8_t  buf[sizeof(ITVL_t) + 64];
//    ITVL_t* cmd = (ITVL_t*)buf;
//
//    switch (medium)
//    {
//        case MEDIMU_EMMC:
//            if(select(bib, site))
//            {
//                /* make to tx shell command "access_blkdev soci <partition> <lba> <len>" */
//                str_len = snprintf((char*)cmd->value, SHELL_STR_WRAPPER_LENGTH, XFER_SHELL_CMD_TEMPLATE(access_blkdev, SHELL_XFER_SOCI, %d, %d, %d), partition, lba, len);
//                /* send command to wait ack */
// 							  inter_boards_txd(cmd, str_len, ITVL_ASCII_TYPE);
// 							  ret = inter_boards_rxd(cmd, 1, ITVL_HEX_TYPE, INTER_COMM_TIMEOUT_SECOND);
// 							  if(ret < 0)
// 									break;
//
//                /* send <soci data> to wait ack */
//                inter_boards_txd(pkg, len, ITVL_HEX_TYPE);
//                ret = inter_boards_rxd(cmd, 1, ITVL_ASCII_TYPE, INTER_COMM_TIMEOUT_SECOND);
//            }
//            break;
//
//        case MEDIMU_E2PROM:
//            if(select(bib, site))
//            {
//                I2C_EE_SetDevAddr(SLT_EEPROM_ADDRESS);
//                I2C_EE_BufferWrite(pkg->value, lba, len);
//            }
//            break;
//
//        case MEDIMU_NAND:
//        case MEDIMU_NOR:
//        case MEDIMU_LPDDR:
//        default:break;
//    }
// 		unselect(bib, site);
//
//    return ret;
// }


/**
 * @brief driver shell command to run without data
 * @param
 * @retval
 */
//static int driver_shell_command(int sock, int trans_dir, int trans_len, int bib, int site, SBW_t * shell)
//{
//	  int i, length, ret = 0;
//
//		if (shell->medium == MEDIMU_SOC)
//		{
//			 if(select(bib, site))
//			 {
//					////reboot()
//					//shell->bib = 0;
//					//shell->site = 0;
//					//snprintf((char*)shell->str, SHELL_STR_WRAPPER_LENGTH, "reboot\r\n");
//
//				  ////copyback(src_part, src_lba, tag_part, tag_lba, len)
//				  //shell->bib = 0;
//				  //shell->site = 0;
//				  //snprintf((char*)shell->str, SHELL_STR_WRAPPER_LENGTH, "copyback %d 0 %d 0 %d\r\n", PARTITION_BOOT1, PARTITION_USER, 32*1024);
//
//				 	////copyback(src_part, src_lba, tag_part, tag_lba, len)
//				  //shell->bib = 0;
//				  //shell->site = 0;
//				  //snprintf((char*)shell->str, SHELL_STR_WRAPPER_LENGTH, "copyback %d 0 %d 0 %d\r\n", PARTITION_USER, PARTITION_BOOT1, 256*1024);
//
//					/* shell transfer without data!!!! */
//				  length = strlen(shell->str);
//					memcpy(gITVL->value, shell->str, length);
//					inter_boards_txd(gITVL, length, ITVL_ASCII_TYPE);
//
//				  if((ret = inter_boards_rxd(gITVL, 1, ITVL_HEX_TYPE, INTER_COMM_TIMEOUT_SECOND)) == TRANSMISSION_OK)
//					{
//							ret = gITVL->value[0];
//					}
//			 }
//			 unselect(bib, site);
//		}
//		else
//		{
//				for (i = 0; shell->str[i] != '\0' && i < strlen(shell->str); ++i)
//						shell(shell->str[i]);
//		}
//
//		return ret;
//}



/**
 * @brief    receive data from the server
 * @param  data, pointer data received
 *         length, request length
 * @retval length received
 */
//static int Client(uint8_t sn)
//{
//
//    CBW_t cbw;
//    CSW_t csw;
//    int32_t ret = TRANSMISSION_STATUS_OK, xfer_len = 0;
//
////#define _DEBUG_OPERA_PACKET_ (3)
//
//#ifdef _DEBUG_OPERA_PACKET_
//
//    memset(&cbw, 0, sizeof(CBW_t));
//    cbw.dCBWSignature = CBW_SIGNATURE;
//    cbw.dCBWTag = 0x12345678;
//    cbw.bCBWLUN = 0;
//
//#if (_DEBUG_OPERA_PACKET_ == 0)
//    //write
//    cbw.bmCBWFlags = TRANSMISSION_DIRECTION_SOCI;
//    cbw.dCBWDataTransferLength = 512 * 4;
//
//    cbw.CDB.opera = WRITE;
//    cbw.CDB.transfer.medium = MEDIMU_EMMC;
//    cbw.CDB.transfer.partition = PARTITION_BOOT1;
//    cbw.CDB.transfer.lba = 0;
//    cbw.CDB.transfer.len = cbw.dCBWDataTransferLength;
//    cbw.CDB.transfer.bib = 0;
//    cbw.CDB.transfer.site = 15;
//
//#elif (_DEBUG_OPERA_PACKET_ == 1)
//    //read
//    cbw.bmCBWFlags = TRANSMISSION_DIRECTION_SICO;
//    cbw.dCBWDataTransferLength = 512 * 4;
//
//    cbw.CDB.opera = READ;
//    cbw.CDB.transfer.medium = MEDIMU_EMMC;
//    cbw.CDB.transfer.partition = PARTITION_BOOT1;
//    cbw.CDB.transfer.lba = 0;
//    cbw.CDB.transfer.len = cbw.dCBWDataTransferLength;
//    cbw.CDB.transfer.bib = 0;
//    cbw.CDB.transfer.site = 15;
//
//#elif (_DEBUG_OPERA_PACKET_ == 4)
//    //inquiry
//    cbw.bmCBWFlags = TRANSMISSION_DIRECTION_SICO;
//    cbw.dCBWDataTransferLength = ALIGN(TL_BURNIN_BOARDS * (sizeof(BIB_INFO_t) + sizeof(uint8_t) * BIB_SKTS), 512);
//
//    cbw.CDB.opera = INQUIRY;
//		cbw.CDB.inquiry.len = cbw.dCBWDataTransferLength;
//		cbw.CDB.inquiry.serial[0] = 0x0001;
//		cbw.CDB.inquiry.serial[1] = 0x0203;
//		cbw.CDB.inquiry.serial[2] = 0x0405;
//		cbw.CDB.inquiry.serial[3] = 0x0607;
//
//#elif (_DEBUG_OPERA_PACKET_ == 2)
//    //soc shell command : "access_mmc sico 0 1 2048"
//    //int str_len = snprintf(gDATABUF, SHELL_STR_WRAPPER_LENGTH, XFER_SHELL_CMD_TEMPLATE(access_mmc, SHELL_XFER_SICO, PARTITION_USER, % d, % d), 1, 2048);
//
//		//mcu shell command : "reset_slt 0 15"
//		int str_len = snprintf((char*)gDATABUF, SHELL_STR_WRAPPER_LENGTH, "reset_slt %d %d\r\n", 0, 14);
//
//    cbw.bmCBWFlags = 0;
//    cbw.dCBWDataTransferLength = 0;
//
//    cbw.CDB.opera = SHELL;
//    cbw.CDB.shell.medium = MEDIMU_MCU;
//    memcpy(cbw.CDB.shell.str, gDATABUF, str_len);
//
//#elif (_DEBUG_OPERA_PACKET_ == 3)
//    //shell command : "change_mode 0 0 0 0"
//
//    cbw.bmCBWFlags = TRANSMISSION_DIRECTION_SOCI;
//    cbw.dCBWDataTransferLength = 0;
//    cbw.CDB.opera = SHELL;
//    cbw.CDB.shell.medium = MEDIMU_SOC;
//		cbw.CDB.shell.bib = 0;
//		cbw.CDB.shell.site = 0;
//    //int str_len = snprintf((char*)gDATABUF, SHELL_STR_WRAPPER_LENGTH, CHANGE_SLT_MODE_CMD(SLT_IDLE_MODE, 0, 15));
//		//int str_len = snprintf((char*)gDATABUF, SHELL_STR_WRAPPER_LENGTH, CHANGE_SLT_MODE_CMD(SLT_TEST_MODE, 0, 15));
//		//int str_len = snprintf((char*)gDATABUF, SHELL_STR_WRAPPER_LENGTH, "copyback %d 0 %d 0 %d\r\n", PARTITION_USER, PARTITION_BOOT1, 256*1024);
//		//int str_len = snprintf((char*)gDATABUF, SHELL_STR_WRAPPER_LENGTH, "copyback %d 0 %d 0 %d\r\n", PARTITION_BOOT1, PARTITION_USER, 256*1024);
//		int str_len = snprintf((char*)gDATABUF, SHELL_STR_WRAPPER_LENGTH, "reboot\r\n");
//    memcpy(cbw.CDB.shell.str, gDATABUF, str_len);
//
//#elif (_DEBUG_OPERA_PACKET_ == 5)
//    //gen data
//    INF("gen data %x:\n", EEPROM_CAPACITY);
//    for(int i=0; i< EEPROM_CAPACITY; ++i)
//		{
//				if((i&31) == 0) INF("\n");
//        INF("0x%04x,", i);
//		}
//
//#endif
//
//    cbw.bCDBLength = sizeof(cbw.CDB);
//    DUMP("CBW:", &cbw, sizeof(CBW_t));
//
//    //ret = driver_shell_command(sn, cbw.bmCBWFlags, cbw.dCBWDataTransferLength, cbw.CDB.shell.bib, cbw.CDB.shell.site, &(cbw.CDB.shell));
//    while(1)
//    {
//         //inquire(cbw.CDB.inquiry.checkbib, cbw.CDB.inquiry.len, cbw.CDB.inquiry.serial, gDATABUF);
//			   //DUMP("INQUIRY", gDATABUF, 512);
//        //uint16_t *ptr = (uint16_t*)gITVL->value;
//        //for(i=0; i<512; ++i)
//        //   ptr[i] = i;
//
//        //TRIG_LOW();
//        //write(0, 15, MEDIMU_E2PROM, PARTITION_USER, 0, 1024, gITVL);
//        //write(0, 15, MEDIMU_EMMC, PARTITION_USER, 0, 1024, gITVL);
//        //TRIG_HIGH();
//
//        //memset(gITVL->value, 0, 1024);
//        //TRIG_LOW();
//        //read(0, 15, MEDIMU_E2PROM, PARTITION_USER, 0, 1024, gITVL);
//        //DUMP("\nREAD1", gITVL, sizeof(ITVL_t) + 512);
//        //read(0, 15, MEDIMU_EMMC, PARTITION_USER, 0, 1024, gITVL);
//        //TRIG_HIGH();
//        //DUMP("\nREAD2", gITVL, sizeof(ITVL_t) + 512);
//    }
//#endif
//
//    //DBG("S%d:%2x\b\b\b\b\b", sn, getSn_SR(sn));
//
//    /* checking whether the command is completed or not, please check the Sn_SR or Sn_IR */
//    switch (getSn_SR(sn))
//    {
//    case SOCK_ESTABLISHED:
//        /**  0x17
//          *  This indicates the status of the connection of socket n
//          *  It changes to SOCK_ESTABLISHED :
//          *  1.When the 'TCP SERVER' processed the SYN packet from the 'TCP CLIENT' during SOCK_LISTEN
//          *  2.When the CONNECT command is successful
//          *  During SOCK_ESTABLISHED, DATA packet can be transferred using SEND or RECV command.
//          *
//          *  Sn_IR(Socket interrupt register checking) :
//          *    Sn_IR_SENDOK(1<<4)     : SEND OK interrupt
//          *    Sn_IR_TIMEOUT(1<<3) : TIMEOUT interrupt
//          *    Sn_IR_RECV(1<<2)     : RECV interrupt
//          *     Sn_IR_DISCON(1<<1)     : DISCON interrupt
//          *     Sn_IR_CON(1<<0)     : CON interrupt
//          *
//          */
//
//        if (getSn_IR(sn) & Sn_IR_CON)
//        {
//            INF("%d:Connected\n", sn);
//            /* clear Sn_IR_CON pending */
//            setSn_IR(sn, Sn_IR_CON);
//        }
//
//        /* data come from w5500 */
//        if (getSn_IR(sn) & Sn_IR_RECV)
//        {
//            setSn_IR(sn, Sn_IR_RECV);
//
//            /**    socket n RX buffer :
//              *   ___max
//              *       |
//              *       |______________Sn_RX_WR[R]
//              *       |            |
//              *    Sn_RX_BUF    |
//              *       |     Sn_RX_RSR[R](the data size received and saved in RX buffer)
//              *       |         |
//              *       |_________|____Sn_RX_RD
//              *   _|_0
//              */
//              /* clear RECV pending to rise INTn up */
//
//            if (getCBW(sn, &cbw) > 0)
//            {
//
//                /* CBW */
//                if (cbw.dCBWSignature != CBW_SIGNATURE)
//                {
//                    return TRANSMISSION_SIGNA_ERR;
//                }
//
//                /* IN/OUT/NULL */
//                switch (cbw.CDB.opera)
//                {
//                    case INQUIRY:
//                        ret = inquire(cbw.CDB.inquiry.checkbib, cbw.CDB.inquiry.len, cbw.CDB.inquiry.serial, gDATABUF);
//                        xfer_len = sendData(sn, gDATABUF, cbw.dCBWDataTransferLength);
//                        break;
//
//                    case WRITE:
//                        xfer_len = recvData(sn, gITVL->value, cbw.dCBWDataTransferLength);
//                        ret = write(cbw.CDB.transfer.bib, cbw.CDB.transfer.site, cbw.CDB.transfer.medium, cbw.CDB.transfer.partition, cbw.CDB.transfer.lba, cbw.CDB.transfer.len, gITVL);
//                        break;
//
//                    case READ:
//                        ret = read(cbw.CDB.transfer.bib, cbw.CDB.transfer.site, cbw.CDB.transfer.medium, cbw.CDB.transfer.partition, cbw.CDB.transfer.lba, cbw.CDB.transfer.len, gITVL);
//                        xfer_len = sendData(sn, gITVL->value, cbw.dCBWDataTransferLength);
//                        break;
//
//                    case SHELL:
//												ret = driver_shell_command(sn, cbw.bmCBWFlags, cbw.dCBWDataTransferLength, cbw.CDB.shell.bib, cbw.CDB.shell.site, &(cbw.CDB.shell));
//                        break;
//
//                    default:break;
//                }
//
//                /* CSW */
//                if ((ret = sendCSW(sn, &csw, cbw.dCBWTag, cbw.dCBWDataTransferLength - xfer_len, ret)) < 0)
//                {
//                    ERR("%d:Send CSW fail\n", sn);
//                    return ret;
//                }
//            }
//        }
//
//        break;
//
//    case SOCK_CLOSE_WAIT:
//        /**  0x1c
//          *  This indicates that socket n received the disconnect-request(FIN package) from the connected peer.
//          *  This is half-closing status and data can be transferred. For full-closing, DISCON command is used.
//          *  But for just-closing, CLOSE command is used.
//          */
//
//        INF("%d:CloseWait\n", sn);
//        if ((ret = disconnect(sn)) != SOCK_OK)
//            return ret;
//        INF("%d:Closed\n", sn);
//        break;
//
//    case SOCK_INIT:
//        /**  0x13
//          *  This indicates that socket n is opened with TCP mode.
//          *  It changes to SOCK_INIT :
//          *  When Sn_MR(P[3:0])='0001' and OPEN command is ordered
//          *  After SOCK_INIT, user can use LISTEN or CONNECT command
//          */
//#ifdef _TCP_CLIENT_
//        if ((ret = connect(sn, gWIZNETINFO.serv, 5000)) < 0)
//            return ret;
//        INF("%d:Connecting ip & port %d\n", sn, ret);
//#else
//        INF("%d:Listen, port [%d]\n", sn, port);
//        if ((ret = listen(sn)) != SOCK_OK)
//            return ret;
//#endif
//        break;
//
//    case SOCK_CLOSED:
//        /**  0x00
//          *  This indicates that socket n is released.
//          *  It changes to SOCK_CLOSED :
//          *  1.When DICON, CLOSE command is ordered
//          *  2.When a timeout occurs
//          */
//
//        INF("%d:LBTStart\n", sn);
//        if ((ret = socket(sn, Sn_MR_TCP, 5000, SF_TCP_NODELAY)) != sn)
//            return ret;
//        break;
//
//    case SOCK_LISTEN:
//        /**  0x14
//          *  This indicates that socket n is operating as 'TCP SERVER' mode and waiting for connection-request(SYN packet) from a peer('TCP CLIENT')
//          */
//
//          /**** temporary status ****/
//    case SOCK_SYNSENT:
//        /**  0x15
//          *  This indicates that socket n sent the connect-request packet(SYN packet) to a peer.
//          *  It is changed from SOCK_INIT to SOCK_ESTABLISHED by CONNECT command.
//          *
//          */
//
//    case SOCK_SYNRECV:
//        /**  0x16
//          *  This indicates that socket n successfully received the connect-request packet(SYN packet) from a peer.
//          *
//          */
//
//    case SOCK_FIN_WAIT:
//    case SOCK_CLOSING:
//    case SOCK_TIME_WAIT:
//        /**  0x18 0x1a 0x1b
//          *  This indicates that socket n is closing.
//          *
//          */
//
//    case SOCK_LAST_ACK:
//        /**  0x1d
//          *  This indicates that socket n is waiting for response(FIN/ACK packet) to the disconnect-request(FIN package) by passive-close.
//          *  It changes to SOCK_CLOSED when socket n received the response successfully, or when timeout occurs(Sn_IR[TIMEOUT]='1')
//          */
//
//    default:
//        break;
//    }
//
//    return 0;
//}

static void do_tcp_client(void)
{
    unsigned short len=0;
	int8_t backk=0;
	uint8_t buff[256]={0};

	switch(getSn_SR(SOCK_TCPC))
	{
		case SOCK_CLOSED:
			socket(SOCK_TCPC,Sn_MR_TCP,local_port,Sn_MR_ND);
			printf("SOCK_CLOSED\r\n");
		  break;

		case SOCK_INIT:
			printf("remote_ip: %d.%d.%d.%d\r\n", remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3]);
			printf("remote_port: %d\r\n", remote_port);
			backk=connect(SOCK_TCPC,remote_ip,remote_port);
			printf("SOCK_INIT\r\n");
			if(backk!=1)
			printf("Socket initial failed and backk is %d\r\n",backk);
		  break;

		case SOCK_ESTABLISHED:

			if(getSn_IR(SOCK_TCPC) & Sn_IR_CON)
			{
				setSn_IR(SOCK_TCPC, Sn_IR_CON);
				printf("SOCK_ESTABLISHED\r\n");
			}

			if(getSn_IR(SOCK_TCPC) & Sn_IR_RECV)
			{
				setSn_IR(SOCK_TCPC, Sn_IR_RECV);
				recv(SOCK_TCPC,buff,256);
				len = strlen((char*)buff);

				process_received_command(SOCK_TCPC,buff,len);

				memset(buff,0,256);
			}
		  break;

		case SOCK_CLOSE_WAIT: 											    	         
			close(SOCK_TCPC);
			printf("SOCK_CLOSE_WAIT\r\n");
		  break;
	}
}



/**
 * @brief main
 */
int main(void)
{
    System_Initialization();
	//BASIC_TIM_Init();
#ifndef _SIMULATION_MTK_SOC_

	printf("Init uart sucessful!\r\n");
	SELECT_CD4052B(0);//0-3四个通道选择用于选择不同的牛角座
	Usart_SendString(USART1,"Init usart1 sucessful!\r\n");

    wizchip_login();

	TCP_Client_Init(local_ip,  subnet,  gateway);

    while (1)
    {
		do_tcp_client();
    }

#else

    handleShell();

#endif

}//end of main()
