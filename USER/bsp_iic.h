#ifndef _BSP_IIC_H_
#define    _BSP_IIC_H_

#include "stm32f10x.h"

/*
* AT24C16 16Kb = 16384bit = 16384/8 B = 2048 B, 16B/page, 128pages, addressing 11bit(3+8) for random
*
* Device Address
* 1 0 1 0 P2 P1 P0 R/W
* 1 0 1 0 0  0  0  0 = 0xA0
* 1 0 1 0 0  0  0  1 = 0xA1
*/

//eeprom
void I2C_EE_Init(void);
void I2C_EE_ByteWrite(u8* pBuffer, u8 WriteAddr);
void I2C_EE_BufferWrite(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite);
void I2C_EE_BufferRead(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead);

//pca9555
void I2C_PCA9555_Write(uint32_t val);

#endif /* __I2C_EE_H */
