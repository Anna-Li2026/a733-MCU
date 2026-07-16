#ifndef __BSP_EEPROM_H
#define __BSP_EEPROM_H
#include "stm32f10x.h"

//#define AT24C64
#define AT24C16
//#define AT24C02

#define MB_EEPROM_ADDRESS   (0xA2)
#define SLT_EEPROM_ADDRESS  (0xA0)

/* memory address */
#if defined AT24C64
#define BYTE_ADDRESS_BITS   (13)
#define PAGE_ADDRESS_BITS   (8)
#elif defined AT24C16
#define BYTE_ADDRESS_BITS   (11)
#define PAGE_ADDRESS_BITS   (7)
#elif defined AT24C02
#define BYTE_ADDRESS_BITS   (8)
#define PAGE_ADDRESS_BITS   (5)
#endif

/* memory organization */
#define PAGE_OFFSET_BITS    (BYTE_ADDRESS_BITS - PAGE_ADDRESS_BITS)
#define EEPROM_PAGESIZE     (1 << PAGE_OFFSET_BITS)
#define EEPROM_PAGES        (1 << PAGE_ADDRESS_BITS)
#define EEPROM_CAPACITY     (1 << BYTE_ADDRESS_BITS)


//EEPROM API
void I2C_EE_SetDevAddr(uint16_t address);
void I2C_EE_ByteWrite(uint8_t* pBuffer, uint16_t WriteAddr);
void I2C_EE_BufferWrite(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);
void I2C_EE_ByteRead(uint8_t *pBuffer);
void I2C_EE_BufferRead(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead);

void I2C_EE_PageWrite(uint8_t* pBuffer, uint16_t WriteAddr, uint8_t NumByteToWrite);

#endif
