
/**
* @file bsp_bib.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief
*
* @details define bus switching of A100_PER3_LPDDR4_32X1_AXP707_V1_0
*
*          mcu pa11/pa12 <- pogopin connector of burnin board
*          mcu iic2      -> pca9555 & eeprom of burnin board
*          mcu uart1     -> uart1 of mtk soc
*          mcu spi2      -> spi nor flash of burnin board for eMMC
*
* @date 2022/11/2
*
* @author heyy
*
* @bug
*
* @Revisions: v1.0
*
*/

#include "bsp_burnin_board.h"



#ifdef _BIB_DEBUG_
#define BI_INF(fmt, ...)   INF(fmt, ##__VA_ARGS__)
#define BI_DBG(fmt, ...)   DBG(fmt, ##__VA_ARGS__)
#define BI_WRN(fmt, ...)   WRN(fmt, ##__VA_ARGS__)
#define BI_ERR(fmt, ...)   ERR(fmt, ##__VA_ARGS__)
#define BI_DUMP(buf,len)   DUMP("EE", buf, len)
#else
#define BI_INF(fmt, ...)
#define BI_DBG(fmt, ...)
#define BI_WRN(fmt, ...)
#define BI_ERR(fmt, ...)
#define BI_DUMP(buf,len)
#endif




extern ITVL_t* gITVL;


const uint32_t SOC[BIB_SKTS][2] = {

     {CS1 ,RST1} , {CS2 ,RST2} , {CS3,RST3}  , {CS4,RST4}  , {CS5,RST5}  , {CS6,RST6}  , {CS7,RST7}  , {CS8,RST8}  ,
     {CS9 ,RST9} , {CS10,RST10}, {CS11,RST11}, {CS12,RST12}, {CS13,RST13}, {CS14,RST14}, {CS15,RST15}, {CS16,RST16},
     {CS17,RST17}, {CS18,RST18}, {CS19,RST19}, {CS20,RST20}, {CS21,RST21}, {CS22,RST22}, {CS23,RST23}, {CS24,RST24},
     {CS25,RST25}, {CS26,RST26}, {CS27,RST27}, {CS28,RST28}, {CS29,RST29}, {CS30,RST30}, {CS31,RST31}, {CS32,RST32},
     {CS33,RST33}, {CS34,RST34}, {CS35,RST35}, {CS36,RST36}, {CS37,RST37}, {CS38,RST38}, {CS39,RST39}, {CS40,RST40},
     {CS41,RST41}, {CS42,RST42}, {CS43,RST43}, {CS44,RST44}, {CS45,RST45}, {CS46,RST46}, {CS47,RST47}, {CS48,RST48},
     {CS49,RST49}, {CS50,RST50}, {CS51,RST51}, {CS52,RST52}, {CS53,RST53}, {CS54,RST54}, {CS55,RST55}, {CS56,RST56},
     {CS57,RST57}, {CS58,RST58}, {CS59,RST59}, {CS60,RST60}, {CS61,RST61}, {CS62,RST62}, {CS63,RST63}, {CS64,RST64},
     {CS65,RST65}, {CS66,RST66}, {CS67,RST67}, {CS68,RST68}, {CS69,RST69}, {CS70,RST70}, {CS71,RST71}, {CS72,RST73},
     {CS73,RST73}, {CS74,RST74}, {CS75,RST75}
};

/**
  * @brief mcu gpio, iic2, uart1 and spi2 bus switching
  * @param
  * @arg
  * @arg
  * @retval
  */
static void cd4052b_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    //default all cd4052b enable
    ENABLE_CD4052B();
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    //default select 0# burnin board
    GPIO_ResetBits(BUS_SWITCH_PORT, BUS_SWITCH_A0 | BUS_SWITCH_A1);
    GPIO_InitStructure.GPIO_Pin = BUS_SWITCH_A0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUS_SWITCH_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = BUS_SWITCH_A1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUS_SWITCH_PORT, &GPIO_InitStructure);
		
}


/**
  * @brief detecting port init
  * @param
  * @arg
  * @arg
  * @retval
  */
static void detect_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = DETECTING_A0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DETECTING_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = DETECTING_A1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DETECTING_PORT, &GPIO_InitStructure);

}


/**
  * @brief check whether tha burnin board is online
  * @param
  * @arg
  * @arg
  * @retval
  */
int check_online(int bib, int enable)
{
    GPIO_WriteBits(BUS_SWITCH_PORT, BUS_SWITCH_A1 | BUS_SWITCH_A0, (bib & 3) << 6);
	  
	  __NOP();
	  __NOP();
	  __NOP();
	  __NOP();
    
	return enable ? (DETECTING_ONLINE == (GPIO_ReadInputData(DETECTING_PORT) & (DETECTING_A1 | DETECTING_A0))) : 1;
}


/**
  * @brief   write 4 bytes to pca555 order : S /W command data0 data1 P
  * @param
  * @arg
  * @arg
  * @retval  null
  */
static void I2C_PCA9555_Write(uint32_t val)
{
    uint8_t* ptr = (uint8_t*)&val;

    /* Send STRAT condition */
    iic_start();

    for(int i=0;i<4;++i)
    {
       iic_send(ptr[i]);
       iic_waitack();
    }
    
    /* Send STOP condition */
    iic_stop();
		
		/* delay 5us */
		__NOP();
		__NOP();
		__NOP();
		__NOP();
}


/**
  * @brief biboard_init
  * @param
  * @arg
  * @arg
  * @retval number of biboard
  */
int biboard_init(int number)
{
		 //MB EEPROM
		 I2C_EE_SetDevAddr(MB_EEPROM_ADDRESS);
		 if(number)
			  I2C_EE_BufferWrite((uint8_t*)&number, 0, 2);
		 I2C_EE_BufferRead((uint8_t*)&number, 0, 2);
	
		 //MB PCA9555
		 I2C_PCA9555_Write(PCA9555_0_IO);
		 I2C_PCA9555_Write(PCA9555_1_IO);
		 I2C_PCA9555_Write(PCA9555_2_IO);
		 I2C_PCA9555_Write(PCA9555_3_IO);
		 I2C_PCA9555_Write(PCA9555_4_IO);
		
		 //disable all pca9555
		 I2C_PCA9555_Write(DIS_PCA9555_0);
		 I2C_PCA9555_Write(DIS_PCA9555_1);
		 I2C_PCA9555_Write(DIS_PCA9555_2);
		 I2C_PCA9555_Write(DIS_PCA9555_3);
		 I2C_PCA9555_Write(DIS_PCA9555_4);
	
	   return number;
}
int32_t mb_eep_read(int bib, int site, uint32_t addr, uint32_t len, uint8_t* buff)
{
	uint32_t ret = select(bib, site);
	
    if(!ret)return ret;
	
	I2C_EE_SetDevAddr(SLT_EEPROM_ADDRESS);
	I2C_EE_BufferRead(buff, addr, len);

    ret = unselect(bib, site);
	
//	printf("[LOG]:select site is %d ,EEPROM Read 0x%X is: ",site+1,addr);
//	for(int e = 0;e<len;e++){
//		printf("%X ",buff[e]);
//	}
//	printf("\r\n");

	return ret;
}

int32_t mb_eep_write(int bib, int site, uint32_t addr, uint32_t len, uint8_t* buff)
{
	uint32_t ret = select(bib, site);
	
	if(!ret)return ret;
	
	I2C_EE_SetDevAddr(SLT_EEPROM_ADDRESS);
	I2C_EE_BufferWrite(buff, addr, len);    
	
    ret = unselect(bib, site);
	
//	printf("[LOG]:select site is %d ,EEPROM Write 0x%X is: ",site+1,addr);
//	for(int e = 0;e<len;e++){
//		printf("%X ",buff[e]);
//	}
//	printf("\r\n");
	return ret;
}

static int eep_memcmp(uint8_t* buff1,uint8_t* buff2,uint32_t len){
	if(memcmp(buff1, buff2, len) == 0){
		printf("[LOG]:!!!!!!!!!!!!EEPROM Write success!!!!!!!!!!!!\r\n");
	}else{
		printf("[ERROR]:!!!!!!!!!!!!EEPROM Write error!!!!!!!!!!!!\r\n");
	}
}
/**
  * @brief 5 pca9555/bib,  4 bib
  * @param
  * @arg
  * @arg
  * @retval
  */
#define TEST_EEPROM_LEN 4
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void slt_init(void)
{
    int bib, site;
	uint32_t addr = 0x00;
	uint32_t addr2 = 0x10;

	uint8_t eep_write_buf[TEST_EEPROM_LEN] = {0};
	uint8_t eep_write_buf2[TEST_EEPROM_LEN] = {0};
	uint8_t eep_read_buf[TEST_EEPROM_LEN] = {0};
	

    for (bib = 0; bib < 1; ++bib)
    {
        if(check_online(bib, 0))
        {
			// MB
			biboard_init(0);
            // MB ALL SITE
			for(site=0; site<BIB_SKTS; ++site)
			{
//				for (int i = 0; i < TEST_EEPROM_LEN; i++) {
//					eep_write_buf[i] = rand();
//				}
				
//				for (int i = 0; i < TEST_EEPROM_LEN; i++) {
//					eep_write_buf2[i] = rand();
//				}
				//AT24C16 write and read
//				mb_eep_write(bib, site, addr, TEST_EEPROM_LEN, eep_write_buf);//				
//				mb_eep_write(bib, site, addr2, TEST_EEPROM_LEN, eep_write_buf2);
				
				mb_eep_read(bib, site, addr, TEST_EEPROM_LEN, eep_read_buf);
				
//				eep_memcmp(eep_write_buf, eep_read_buf, TEST_EEPROM_LEN);
				memset(eep_read_buf,0,TEST_EEPROM_LEN);
				
				mb_eep_read(bib, site, addr2, TEST_EEPROM_LEN, eep_read_buf);				
				
//				eep_memcmp(eep_write_buf, eep_read_buf, TEST_EEPROM_LEN);
				memset(eep_read_buf,0,TEST_EEPROM_LEN);
            }
        }
        else
        {
            BI_WRN("slot#%d null\n", bib);
        }
    }
}


void burnin_init(void)
{
    cd4052b_init();
    detect_init();
    iic_sim_init();                         //Init i2c to access eeprom and pca9555
    bsp_usart1_init();                      //usart1 communication between stm32 and mt6757 by DMA
    slt_init();								//change the site
}



/**
  * @brief unselect site
  * @param
  * @arg
  * @arg
  * @retval
  */
int unselect(int bib, int site)
{
	  if((check_online(bib, 0) == 0) || (site > BIB_SKTS))
				return 0;
		
		//MB PCA9555
		I2C_PCA9555_Write(SOC[site][0] | DISABLE_SITE_MARK);
		
//	  BI_DBG("%s:%08x\r\n", __FUNCTION__, SOC[site][0] | DISABLE_SITE_MARK);
		return 1;
}

/**
  * @brief select site of bib, elapsed 2.52ms per site
  * @param
  * @arg
  * @arg
  * @retval
  */
int select(int bib, int site)
{
    if (check_online(bib, 0) == 0 || (site > BIB_SKTS))
        return 0;
		
		//init pca9555 of mother board
		I2C_PCA9555_Write(PCA9555_0_IO);
		I2C_PCA9555_Write(PCA9555_1_IO);
		I2C_PCA9555_Write(PCA9555_2_IO);
		I2C_PCA9555_Write(PCA9555_3_IO);
		I2C_PCA9555_Write(PCA9555_4_IO);
    I2C_PCA9555_Write(SOC[site][0]);
		
//	  BI_DBG("%s:%08x\r\n", __FUNCTION__, SOC[site][0]);
    return 1;
}



/**
  * @brief reset soc, elapsed 2.37ms per site
  * @param
  * @arg
  * @arg
  * @retval
  */
int reset_soc(int bib, int site)
{
    if (check_online(bib, 0) == 0 || (site > BIB_SKTS))
        return 0;
		
		//init pca9555 of mother board
		I2C_PCA9555_Write(PCA9555_0_IO);
		I2C_PCA9555_Write(PCA9555_1_IO);
		I2C_PCA9555_Write(PCA9555_2_IO);
		I2C_PCA9555_Write(PCA9555_3_IO);
		I2C_PCA9555_Write(PCA9555_4_IO);
    I2C_PCA9555_Write(SOC[site][1]);
		delay_ms(300);																				//MT6771 RESETB release 30ms (default)
		I2C_PCA9555_Write(SOC[site][1] | DISABLE_SITE_MARK);
		
		BI_DBG("%s:%08x 300ms release %08x\n", __FUNCTION__, SOC[site][1], SOC[site][1] | DISABLE_SITE_MARK);
    return 1;
}



/**
 * @brief    inter transfer between boards
 * @param  rxd, rx data form peer board
 *         length, the length of rxd->value
 *				 timeout, second
 * @retval
 */
int inter_boards_rxd(ITVL_t* rxd, int length, int bType, int timeout)
{
		if(usart1_rx_request(rxd, sizeof(ITVL_t) + length, timeout))
			return TRANSMISSION_TIMEOUT;
	
    if (ITVL_HEAD != rxd->wHead || 0 != rxd->wCheckSum)
        return TRANSMISSION_CRC_ERR;

    return TRANSMISSION_OK;
}



/**
 * @brief    inter transfer between boards
 * @param  txd, tx data to peer board
 * @param  value_length, the length of txd->value
 * @retval
 */
int inter_boards_txd(ITVL_t* txd, int length, int bType)
{
    //INF("#%d-%d txd %d\n", bib, site, length);
    
    /* first tx 8byte head */
    txd->wHead = ITVL_HEAD;
    txd->wID = 0;
    txd->bType = bType & 1;
    txd->wLength = length & 0xffff;
	  txd->wCheckSum = 0;

    /* tx data */
    return usart1_tx_request(txd, sizeof(ITVL_t) + length);
}
