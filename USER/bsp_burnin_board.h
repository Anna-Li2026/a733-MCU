/**
* @file bsp_burnin_board.h
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief
*
* @details define switch of burnin board
*
* @date 2022/11/2
*
* @author Jeff.he
*
* @bug
*
* Revisions: v1.0
*
*/

#ifndef __BSP_BURNIN_BOARD_H
#define __BSP_BURNIN_BOARD_H

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "string.h"
#include "protocol.h"
#include "trace.h"
#include "bsp_sim_iic.h"
#include "bsp_eeprom.h"
#include "bsp_inter_xfer.h"
#include "burnin_board_map.h"
#include "bsp_inter_xfer.h"

//define bus switching cd4052b port
#define    	ENABLE_CD4052B()         do { GPIO_SetBits(GPIOA, GPIO_Pin_8); }while(0)
#define    	DISABLE_CD4052B()        do { GPIO_ResetBits(GPIOA, GPIO_Pin_8); }while(0)
//#define    	ENABLE_TM1109()        	do { GPIO_SetBits(GPIOA, GPIO_Pin_8); }while(0)
#define		SELECT_CD4052B(ch)		 do{ENABLE_CD4052B(); GPIO_WriteBits(BUS_SWITCH_PORT, BUS_SWITCH_A1 | BUS_SWITCH_A0,(ch&0x03)<<6);}while(0)

//define bus(iic, uart, spi, detect) switch
#define    BUS_SWITCH_PORT          (GPIOC)
#define    BUS_SWITCH_A0            (GPIO_Pin_6)
#define    BUS_SWITCH_A1            (GPIO_Pin_7)

//define detecting the insertion of burnin board port
#define    DETECTING_PORT           (GPIOA)
#define    DETECTING_A0             (GPIO_Pin_11)
#define    DETECTING_A1             (GPIO_Pin_12)
#define    DETECTING_ONLINE         (0<<11)

void burnin_init(void);
int check_online(int bib, int enable);
int biboard_init(int number);
int unselect(int bib, int site);
int select(int bib, int site);
int reset_soc(int bib, int site);
int inter_boards_txd(ITVL_t* txd, int length, int bType);
int inter_boards_rxd(ITVL_t* rxd, int length, int bType, int timeout);

int32_t mb_eep_read(int bib, int site,uint32_t addr,uint32_t len, uint8_t* buff);
int32_t mb_eep_write(int bib, int site,uint32_t addr,uint32_t len, uint8_t* buff);

#endif
