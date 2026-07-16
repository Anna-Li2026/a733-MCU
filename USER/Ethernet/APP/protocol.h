/**
* @file protocol.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief
*
* @details define the protocol that above TCP/UDP protocol
*
* @date 2022/10/17
*
* @author Jeff.he
*
* @bug
*
* Revisions: v1.2
*
*/

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#ifdef __cplusplus
namespace artmem {
    extern "C" {
#endif
#include <stdint.h>


        /* define the structures are organized in memory */
#ifdef _MSC_VER
/* Windows MSVC */
#define __PACKED__
#pragma pack(push, 1)
#else
#pragma diag_suppress 2795
/* Linux GCC/ARMCC */
#define __PACKED__                          __attribute__((packed))
#endif

/* define socket for transfer */
#define SOCK_TRANSFER                       (0)

/* define SOC uart port */
#define INTER_COMM_UART                     (UART2)
#define INTER_COMM_BOUND                    (115200)
#define GPIO_UART_URXD1_PIN                 (GPIO19 | 0x80000000)
#define GPIO_UART_URXD1_PIN_M_URXD          (GPIO_MODE_02)
#define GPIO_UART_UTXD1_PIN                 (GPIO20 | 0x80000000)
#define GPIO_UART_UTXD1_PIN_M_UTXD          (GPIO_MODE_02)

/* define inter-communication by uart */
#define INTER_COMM_DATA_BITS                USART_WordLength_8b
#define INTER_COMM_STOP_BITS                USART_StopBits_1
#define INTER_COMM_PARITY_BITS              USART_Parity_No
#define INTER_FLOW_CTRL                     USART_HardwareFlowControl_None
#define INTER_COMM_TIMEOUT_SECOND           (10)												    

/* define status */
#define TRANSMISSION_OK                     (0)
#define TRANSMISSION_SIGNA_ERR              (-1)
#define CBW_PACKET_SIZE_ERR                 (-2)
#define CSW_SEND_ERR                        (-3)
#define TRANSMISSION_CRC_ERR                (-4)
#define TRANSMISSION_BIB_ERR                (-5)
#define TRANSMISSION_TIMEOUT								(-6)

/* define transport layer basic parameter */
#define MAX_TRANSMISSION_LENGTH             (4096 + sizeof(ITVL_t))         //max lenght of single transmission task

#define CBW_SIGNATURE                       (0x19491001)                    //CBW tag
#define CSW_SIGNATURE                       (0x20171001)                    //CSW tag
#define CDB_LENGTH													(48)
#define	SHELL_STR_WRAPPER_LENGTH						(CDB_LENGTH - 1 - 3)
#define TRANSMISSION_DIRECTION_SOCI         (0x00)                          //server output client input
#define TRANSMISSION_DIRECTION_SICO         (0x80)                          //server input    client output
#define TRANSMISSION_LUN                    (0x00)                          //default socket 0
#define TRANSMISSION_STATUS_OK              (0x00)                          //transmission task ok
#define TRANSMISSION_STATUS_FAIL            (0x01)                          //transmission task occure error

/* define private protocol command operation */
#define DEFAULT_VALUE_FILLED								(0xCC)

/* define operation code */
#define INQUIRY                             (0x12)
#define WRITE                               (0xAA)
#define READ                                (0xA8)
#define SHELL                               (0xA0)

/* define medimu accessed */
#define MEDIMU_SOC                          (0)
#define MEDIMU_MCU                          (1)
#define MEDIMU_EMMC                         (2)
#define MEDIMU_E2PROM                       (3)
#define MEDIMU_LPDDR                        (4)
#define MEDIMU_NAND                         (5)
#define MEDIMU_NOR                          (6)

/* define partition of medimu */
#define PARTITION_USER                      0
#define PARTITION_BOOT1                     1
#define PARTITION_BOOT2                     2

/* define ITVL packet parameter */
/* number */
#define ITVL_HEAD                           0x55AA
#define ITVL_ASCII_TYPE                     0
#define ITVL_HEX_TYPE                       1
/* str */
#define SHELL_XFER_SOCI                     soci
#define SHELL_XFER_SICO                     sico

#define SHELL_XFER_SOCI_STR                 "soci"
#define SHELL_XFER_SICO_STR                 "sico"

/* symbol s is converted to text "s" */
#define _T(s)                               #s

/* define shell command template for transfer */
/* shell command transfer template : "<name> <xfer_io> <partition> <lba> <len>" */
#define XFER_SHELL_CMD_TEMPLATE(name, io, part, lba, len) \
        (_T(name)##_T(\x20)##_T(io)##_T(\x20)##_T(part)##_T(\x20)##_T(lba)##_T(\x20)##_T(len)##_T(\xd)##_T(\xa))

/* n align a */
#define ALIGN(n, a)                          (((n) + (a) - 1) & (~((a) - 1)))

/* define Control Panel Item and Log at EEPROM or BLKDEV(after 6GB) for lpddr burnin test item and logger */
#define CPL_SIGN                             (0x4C4F4700)
#define CPL_LBA                              (0x00C00000)
#define CPL_MAX_LEN                          (3584)

#define LOG_SIGN                             (0x43504C00)
#define LOG_LBA                              (0x00000000)
#define LOG_MAX_LEN                          (512)

/* define the soc mode */
#define SLT_IDLE_MODE                        0
#define SLT_TEST_MODE                        1

/* define soc support string of shell command */
#define CHANGE_SLT_MODE_CMD(mode, bib, site)	XFER_SHELL_CMD_TEMPLATE(change_mode, mode, bib, site, 0)


/* inquiry descriptor block */
typedef struct
{
	  uint8_t     checkbib : 1;																									 //check bib online or not
	  uint8_t     :7;
    uint32_t    len;                                                           //IN transfer data length
	  uint16_t    serial[4];																							 		   //slot#0~3 serial number
} __PACKED__ INQUIRY_t;


/* inquiry information format */
typedef struct
{
	 uint8_t 		  status;
	 char					version[16 -sizeof(uint8_t)];
} __PACKED__ SLT_INFO_t;


typedef struct
{
    uint16_t    serial;                                                        //current bi board serial number
    uint16_t    sites;                                                         //current bi board total sites
    uint16_t    size;                                                          //size of package
	  char				st_version[24 - sizeof(uint16_t)*3];
	  SLT_INFO_t  slt[0];																												 //information per slt
} __PACKED__ BIB_INFO_t;


/* transfer data command  descriptor block */
typedef struct
{
    uint8_t     partition : 4;                                                 //partition of medium
    uint8_t     medium : 4;                                                    //access medium
    uint32_t    lba;                                                           //logic block address (Little Endian)
    uint32_t    len;                                                           //transfer length(Little Endian)
    uint8_t     bib;                                                           //slot of bib
    uint8_t     site;                                                          //index of site of bib
} __PACKED__ TRANSFER_t;


/* shell block wrapper */
typedef struct
{
    uint8_t : 5;                                                               //reserve
    uint8_t     medium : 3;                                                    //access medium
	  uint8_t     bib;																													 //slot of bib
	  uint8_t     site;																													 //index of site of bib
    char        str[SHELL_STR_WRAPPER_LENGTH];                                 //shell packet
} __PACKED__ SBW_t;

#define CBW_LENGTH (1024)
//#define CDB_LENGTH ((CBW_LENGTH-4*sizeof(uint8_t)-3*sizeof(uint32_t)-1*sizeof(uint64_t))/2)
#define CDBSHELL_LENGTH (CDB_LENGTH - 4*sizeof(uint8_t)-1* sizeof(uint32_t))

/* define command descriptor block */
typedef struct
{
    uint8_t     opera;                                                          //command operation code

    union {
        uint8_t     _[CDB_LENGTH - 1];                                          //structure placeholder, don't use
        INQUIRY_t   inquiry;                                                    //inquiry
        TRANSFER_t  transfer;                                                   //transfer IN/OUT
        SBW_t       shell;                                                      //shell
    };
} __PACKED__ CDB_t;                                                             //48byte


/* define command block wrapper */
typedef struct
{
    uint32_t    dCBWSignature;                                                  //CBW signature
    uint32_t    dCBWTag;                                                        //current CBW tag
    uint32_t    dCBWDataTransferLength;                                         //IN/OUT transfer length
    uint8_t     bmCBWFlags;                                                     //transfer direction
    uint8_t     bCBWLUN;                                                        //lun
    uint8_t     bCDBLength;                                                     //valid length of CDB
    CDB_t       CDB;                                                            //command descriptor block
} __PACKED__ CBW_t;


/* define command status wrapper */
typedef struct
{
    uint32_t    dCSWSignature;                                                  //CSW signature
    uint32_t    dCSWTag;                                                        //curretn CSW tag ablout
    uint32_t    dCSWDataResidue;                                                //transfer data residue
    uint8_t     bCSWStatus;                                                     //transfer status
} __PACKED__ CSW_t;


/* define inter-board transmission packet */
typedef struct
{
    uint16_t    wHead;                                                          //packet head 0x55AA
    uint16_t    wID : 8;                                                        //pcaket id
    uint16_t    bType : 1;                                                      //ascii or hex
    uint16_t : 7;
    uint16_t    wCheckSum;                                                      //checksum of packet
    uint16_t    wLength;                                                        //valid length of value
    uint8_t     value[0];                                                       //data content
} __PACKED__ ITVL_t;


/* define Control Panel Item(CPL) */
typedef struct
{
    uint32_t    sign;
    uint32_t    checksum;
    uint16_t    size;
    uint8_t     items;
    uint8_t     res[64 - sizeof(uint32_t)*2 - sizeof(uint16_t)*1 - sizeof(uint8_t)*1];
    char        content[0];
} __PACKED__ CPL_t;


/* define logger */
typedef struct
{
    uint16_t    idx;
    uint16_t    cycle;
    uint16_t    temp;
    uint16_t    error;
} __PACKED__ item_t;


typedef struct
{
    uint32_t    sign;
    uint32_t    checksum;
    uint16_t    size;
    uint16_t    pos;
    uint8_t     next;
    uint8_t     items;
    uint8_t     res[32 - sizeof(uint32_t)*2 - sizeof(uint16_t)*2 - sizeof(uint8_t)*2];
    item_t      item[0];
} __PACKED__ LOG_t;


#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifdef __cplusplus
    }/* ?extern "C"? */
}/* ?namespace?*/
#endif

#endif
