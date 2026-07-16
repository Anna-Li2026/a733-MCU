/**
* @file bsp_eeprom.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief eeprom driver
*
* @details
*
*
* @date 2022/11/14
*
* @author heyy
*
* @bug
*
* Revisions: v1.0
*
*/
#include "bsp_sim_iic.h"
#include "bsp_eeprom.h"
#include "trace.h"


static uint16_t EEPROM_ADDRESS = 0xA0;


/**
  * @brief   I2C_EE_SetDevAddr
  * @param   set device address
  * @arg     
  * @arg
  * @retval
  */
void I2C_EE_SetDevAddr(uint16_t address)
{
    EEPROM_ADDRESS = address;
}


/**
  * @brief  Wait for EEPROM Standby state
  * @param
  * @retval
  */
static void I2C_EE_WaitEepromStandbyState(void)
{
#if 0
    
    //TRIG_LOW();
    
    /* dummy byte write command */
    do
    {
        /* Send START condition */
        iic_start();
        /* Send EEPROM address for write */
        iic_send(EEPROM_ADDRESS | IIC_WR);
        
        __NOP();
        __NOP();
        __NOP();
    } while (!iic_waitack());

    /* STOP condition */
    iic_stop();
    
    //TRIG_HIGH();
    
#else
    
    delay_ms(5);
    
#endif

}



/**
  * @brief   byte write
  * @param   S device ACK A15-A8 ACK A7-A0 ACK D7-D0 ACK P
  * @arg     pBuffer : byte to write
  * @arg
  * @retval
  */
void I2C_EE_ByteWrite(uint8_t *pBuffer, uint16_t WriteAddr)
{
    /* wait tWR */
    I2C_EE_WaitEepromStandbyState();
    
    /* Send STRAT condition */
    iic_start();

//    /* Send EEPROM address for write */
//    iic_send(EEPROM_ADDRESS | IIC_WR);
//    iic_waitack();

//    /* Send the EEPROM's internal address to write to */
//#if (BYTE_ADDRESS_BITS > 8)
//    iic_send((WriteAddr>>8) & 0xff);
//    iic_waitack();
//#endif
//    
//    iic_send(WriteAddr & 0xff);
#if defined(AT24C16)
    uint8_t dev_addr = (EEPROM_ADDRESS & 0xFE) | ((WriteAddr >> 8) & 0x01);
    iic_send(dev_addr | IIC_WR);
#else
    iic_send(EEPROM_ADDRESS | IIC_WR);
#endif
    iic_waitack();
#if (BYTE_ADDRESS_BITS > 8)
    #if defined(AT24C16)
        iic_send(WriteAddr & 0xff);
    #else
        iic_send((WriteAddr>>8) & 0xff);
        iic_waitack();
        iic_send(WriteAddr & 0xff);
    #endif
#else
    iic_send(WriteAddr & 0xff);
#endif

    iic_waitack();
    
    /* Send the byte to be written */
    iic_send(*pBuffer);
    iic_waitack();

    /* Send STOP condition */
    iic_stop();
    

}



/**
  * @brief   current address read
  * @param   S device ACK data NACK P
  * @arg     pBuffer : byte to write
  * @arg
  * @retval
  */
void I2C_EE_ByteRead(uint8_t *pBuffer)
{
    *pBuffer = 0xFF;
    
    /* Send STRAT condition */
    iic_start();

    /* Send EEPROM address for write */
    iic_send(EEPROM_ADDRESS | IIC_RD);
    iic_waitack();

    /* Send the byte to be written */
    *pBuffer = iic_recv();
    iic_nack();

    /* Send STOP condition */
    iic_stop();
}



/**
  * @brief   page write
  * @param S DEVICE A15-A8 A7-A0 DATA P
  * @arg
  * @arg
  * @arg
  * @retval
  */
void I2C_EE_PageWrite(uint8_t* pBuffer, uint16_t WriteAddr, uint8_t NumByteToWrite)
{
    /* wait tWR max 5ms */
    I2C_EE_WaitEepromStandbyState();
    
    /* Send START condition */
    iic_start();

#if defined(AT24C16)
	uint8_t page_addr = (WriteAddr >> 8) & 0x07;  // A10,A9,A8
    uint8_t dev_addr = (EEPROM_ADDRESS & 0xF0) | (page_addr << 1);
    iic_send(dev_addr | IIC_WR);
#else
	uint8_t dev_addr = (EEPROM_ADDRESS & 0xFE) | ((WriteAddr >> 8) & 0x01);
    iic_send(EEPROM_ADDRESS | IIC_WR);
#endif
    iic_waitack();
#if (BYTE_ADDRESS_BITS > 8)
    #if defined(AT24C16)
        iic_send(WriteAddr & 0xff);
    #else
        iic_send((WriteAddr>>8) & 0xff);
        iic_waitack();
        iic_send(WriteAddr & 0xff);
    #endif
#else
    iic_send(WriteAddr & 0xff);
#endif

    iic_waitack();
	
    /* While there is data to be written */
    while(NumByteToWrite--)
    {
        /* Send the current byte */
        iic_send(*pBuffer++);
        iic_waitack();
    }

    /* Send STOP condition */
    iic_stop();
    
}



/**
  * @brief  write data of the pBuffer to the E2PROM
  * @param
  * @arg    pBuffer : buffer pointer
  * @arg    WriteAddr : address
  * @arg    NumByteToWrite : number byte to write
  * @retval
  */
void I2C_EE_BufferWrite(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
{
    int offset, tranlen, lastlen;
    
    //printf("@pBuffer %x, WriteAddr %d, NumByteToWrite %d\r\n", (uint32_t)pBuffer, WriteAddr, NumByteToWrite);

    for(offset = WriteAddr & (EEPROM_PAGESIZE - 1), lastlen = NumByteToWrite; lastlen > 0 ; offset = 0)
    {
        tranlen = (lastlen > (EEPROM_PAGESIZE - offset)) ? (EEPROM_PAGESIZE - offset): lastlen;
        
        //printf("offset %d , pBuffer %x, WriteAddr %d, tranlen %d\r\n", offset, (uint32_t)pBuffer, WriteAddr, tranlen);
        
        I2C_EE_PageWrite(pBuffer,  WriteAddr, tranlen);
    
        lastlen -= tranlen;
        WriteAddr += tranlen;
        pBuffer += tranlen;
    }
}



/**
  * @brief   random read
  * @param   S device ACK A15-A8 ACK A7-A0 ACK S device ACK data NACK P
  * @arg pBuffer : pointer of bulk data
  * @arg WriteAddr: address of eeprom
  * @arg NumByteToWrite: request number bytes
  * @retval
  */
void I2C_EE_BufferRead(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
{
    I2C_EE_WaitEepromStandbyState();

    /* Send START condition */
    iic_start();

#if defined(AT24C16)
	uint8_t page_addr = (ReadAddr >> 8) & 0x07;   // ?? A10,A9,A8
    uint8_t dev_addr = (EEPROM_ADDRESS & 0xF0) | (page_addr << 1);
    iic_send(dev_addr | IIC_WR);
#else
    iic_send(EEPROM_ADDRESS | IIC_WR);
#endif
    iic_waitack();

#if (BYTE_ADDRESS_BITS > 8)
    #if defined(AT24C16)
        iic_send(ReadAddr & 0xff);
    #else
        iic_send((ReadAddr>>8) & 0xff);
        iic_waitack();
        iic_send(ReadAddr & 0xff);
    #endif
#else
    iic_send(ReadAddr & 0xff);
#endif

    iic_waitack();
    
    /* Send STRAT condition a second time */
    iic_start();

    /* Send EEPROM address for read */
#if defined(AT24C16)
	dev_addr = (EEPROM_ADDRESS & 0xF0) | (page_addr << 1);
    iic_send(dev_addr | IIC_RD);
#else
    iic_send(EEPROM_ADDRESS | IIC_RD);
#endif

    iic_waitack();

    while(NumByteToRead--)
    {
        *pBuffer++ = iic_recv();
        (NumByteToRead) ? iic_ack() : iic_nack();
    }
    
    iic_stop();
}
