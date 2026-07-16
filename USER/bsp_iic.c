/**
* @file bsp_iic.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief iic2 eeprom(AT24C16) application
*
* @details
*
*
* @date 2022/9/14
*
* @author heyy
*
* @bug
*
* Revisions: v1.0
*
*/

#include "bsp_iic.h"
#include "trace.h"

/* select I2C channel !!*/
#define _CURRENT_I2C_IS_        (2)


#ifdef _EEPROM_DEBUG_
#define EE_INF(fmt, ...)   INF(fmt, ##__VA_ARGS__)
#define EE_DBG(fmt, ...)   DBG(fmt, ##__VA_ARGS__)
#define EE_WRN(fmt, ...)   WRN(fmt, ##__VA_ARGS__)
#define EE_ERR(fmt, ...)   ERR(fmt, ##__VA_ARGS__)
#define EE_DUMP(buf,len)   DUMP("EE", buf, len)
#else
#define EE_INF(fmt, ...)
#define EE_DBG(fmt, ...)
#define EE_WRN(fmt, ...)
#define EE_ERR(fmt, ...)
#define EE_DUMP(buf,len)
#endif


/* STM32 I2C1 or I2C2 */
#if (_CURRENT_I2C_IS_ == 1)
#define I2Cx                    (I2C1)
////AT24C02
#define EEPROM_Block0_ADDRESS    0xA0   /* E2 = 0 */
#define I2C_PageSize            (8)
#else
#define I2Cx                    (I2C2)
//AT24C64 organized : 256pages, 32bytes/page
// write opeara:
// 1.byte write : S device W address data P
// 2.page write : S device W address data0 ... data31 P
// read opeara:
// 1.byte read  : S device R data P
// 2.ran read : S device W address S device R datan P(host no ack)
// 3.seq read : S device R data0 ... data31 P(host ack)
#define EEPROM_Block1_ADDRESS     0xA2   /* E2 = 0 */
#define I2C_PageSize              (8)
#endif


/* STM32 I2C Fast Mode */
#define I2C_Speed                (400000)
/* eeprom address */
#define I2C2_OWN_ADDRESS7        (0x0A)


static uint16_t EEPROM_ADDRESS;


///**
//  * @brief  release the iic bus
//  * @param
//  * @retval
//  */
//static void I2C_SCL_Pulldown()
//{
//    GPIO_InitTypeDef  GPIO_InitStructure;
//    
//    /* force iic scl pull down to release iic bus */
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//    GPIO_Init(GPIOB, &GPIO_InitStructure);
//    GPIO_ResetBits(GPIOB, GPIO_Pin_10);
//    
//    /* reconfig scl */
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//    GPIO_Init(GPIOB, &GPIO_InitStructure);
//    
//}


/**
  * @brief  I2C2 I/O config
  * @param
  * @retval
  */
static void I2C_GPIO_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /* PORT and I2C2 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

#if (_CURRENT_I2C_IS_ == 1)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    /* PB6-I2C1_SCL PB7-I2C1_SDA */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
#else
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    
    /* force scl low */
    GPIO_ResetBits(GPIOB, GPIO_Pin_10);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* PB10-I2C2_SCL PB11-I2C2_SDA */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

#endif
}


static void I2C_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* config IT and NVIC */
#if (_CURRENT_I2C_IS_ == 1)
    NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
#else
    NVIC_InitStructure.NVIC_IRQChannel = I2C2_ER_IRQn;
#endif

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}



/**
  * @brief  I2C work mode
  * @param
  * @retval
  */
static void I2C_Mode_Config(void)
{
    I2C_InitTypeDef  I2C_InitStructure;

    /* I2C mode */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;

    /* high level data stabilize, low level change */
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;

    I2C_InitStructure.I2C_OwnAddress1 = I2C2_OWN_ADDRESS7;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;

    /* I2C adressing mode */
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    /* I2C speed */
    I2C_InitStructure.I2C_ClockSpeed = I2C_Speed;

    /* I2C init */
    I2C_Init(I2Cx, &I2C_InitStructure);
    I2C_ITConfig(I2Cx, I2C_IT_ERR, ENABLE);
    
    EE_INF("IIC%d CR1 %x, CR2 %x, OAR1 %x, OAR2 %x, SR1 %x, SR2 %x, CCR %x, TRISE %x\n", \
    _CURRENT_I2C_IS_, I2Cx->CR1, I2Cx->CR2, I2Cx->OAR1, I2Cx->OAR2, I2Cx->SR1, I2Cx->SR2, I2Cx->CCR, I2Cx->TRISE);

    /* enable I2C */
    I2C_Cmd(I2Cx, ENABLE);
}


#if ( _CURRENT_I2C_IS_ == 1 )
void I2C1_ER_IRQHandler(void)
#else
void I2C2_ER_IRQHandler(void)
#endif
{
    TRIG_LOW();
    
    //ERR("I2C SR1 %x\n" , I2C_ReadRegister(I2Cx, I2C_Register_SR1));
    if(I2C_GetITStatus(I2Cx, I2C_IT_AF))
    {
        I2C_ClearITPendingBit(I2Cx, I2C_IT_AF);
        I2C_GenerateSTOP(I2Cx, ENABLE);
        ERR("I2C%d AF\n", _CURRENT_I2C_IS_);
    }
    
    if(I2C_GetITStatus(I2Cx, I2C_IT_TIMEOUT)) 
    {
        I2C_ClearITPendingBit(I2Cx, I2C_IT_TIMEOUT);
        I2C_SoftwareResetCmd(I2Cx, ENABLE);
        ERR("I2C%d TO\n", _CURRENT_I2C_IS_);
    }
    
    TRIG_HIGH();
}


/**
  * @brief  Init I2C E2PROM
  * @param
  * @retval
  */
void I2C_EE_Init(void)
{

    I2C_GPIO_Config();
    I2C_NVIC_Config();
    I2C_Mode_Config();
    

#ifdef EEPROM_Block0_ADDRESS
    EEPROM_ADDRESS = EEPROM_Block0_ADDRESS;
#endif

#ifdef EEPROM_Block1_ADDRESS  
    EEPROM_ADDRESS = EEPROM_Block1_ADDRESS;
#endif

#ifdef EEPROM_Block2_ADDRESS  
    EEPROM_ADDRESS = EEPROM_Block2_ADDRESS;
#endif

#ifdef EEPROM_Block3_ADDRESS  
    EEPROM_ADDRESS = EEPROM_Block3_ADDRESS;
#endif

}


/**
  * @brief  Wait for EEPROM Standby state
  * @param
  * @retval
  */
static void I2C_EE_WaitEepromStandbyState(void)
{
    vu16 SR1_Tmp = 0;

    do
    {
        /* Send START condition */
        I2C_GenerateSTART(I2Cx, ENABLE);
        /* Read I2C1 SR1 register */
        SR1_Tmp = I2C_ReadRegister(I2Cx, I2C_Register_SR1);
        /* Send EEPROM address for write */
        I2C_Send7bitAddress(I2Cx, EEPROM_ADDRESS, I2C_Direction_Transmitter);
    } while (!(I2C_ReadRegister(I2Cx, I2C_Register_SR1) & 0x0002));

    /* Clear AF flag */
    I2C_ClearFlag(I2Cx, I2C_FLAG_AF);
    /* STOP condition */
    I2C_GenerateSTOP(I2Cx, ENABLE);
}


/**
  * @brief   write a byte to EEPROM
  * @param   S device /W address data P
* @arg     pBuffer : byte to write
  * @arg
  * @retval
  */
void I2C_EE_ByteWrite(u8* pBuffer, u8 WriteAddr)
{
    /* Send STRAT condition */
    I2C_GenerateSTART(I2Cx, ENABLE);

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(I2Cx, EEPROM_ADDRESS, I2C_Direction_Transmitter);

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    /* Send the EEPROM's internal address to write to */
    I2C_SendData(I2Cx, WriteAddr);

    /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    
    /* Send the byte to be written */
    I2C_SendData(I2Cx, *pBuffer);

    /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /* Send STOP condition */
    I2C_GenerateSTOP(I2Cx, ENABLE);
}


/**
  * @brief   write page that repeatable writed but the number of bytes can't exceed the page size
  * @param
  * @arg
  * @arg
  * @arg
  * @retval
  */
static void I2C_EE_PageWrite(u8* pBuffer, u8 WriteAddr, u8 NumByteToWrite)
{
    /* sda or scl always is at low */
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

    /* Send START condition */
    I2C_GenerateSTART(I2Cx, ENABLE);

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(I2Cx, EEPROM_ADDRESS, I2C_Direction_Transmitter);

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    /* Send the EEPROM's internal address to write to */
    I2C_SendData(I2Cx, WriteAddr);

    /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /* While there is data to be written */
    while(NumByteToWrite--)
    {
        /* Send the current byte */
        I2C_SendData(I2Cx, *pBuffer++);
        /* Test on EV8 and clear it */
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }
    
    /* Send STOP condition */
    I2C_GenerateSTOP(I2Cx, ENABLE);
}


/**
  * @brief   read a bulk data from eeprom
  * @param
  * @arg pBuffer : pointer of bulk data
  * @arg WriteAddr: address of eeprom
  * @arg NumByteToWrite: request number bytes
  * @retval
  */
void I2C_EE_BufferRead(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead)
{
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

  /* Send START condition */
    I2C_GenerateSTART(I2Cx, ENABLE);

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(I2Cx, EEPROM_ADDRESS, I2C_Direction_Transmitter);

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    /* Clear EV6 by setting again the PE bit */
    I2C_Cmd(I2Cx, ENABLE);

    /* Send the EEPROM's internal address to write to */
    I2C_SendData(I2Cx, ReadAddr);

    /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    
    /* Send STRAT condition a second time */
    I2C_GenerateSTART(I2Cx, ENABLE);

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send EEPROM address for read */
    I2C_Send7bitAddress(I2Cx, EEPROM_ADDRESS, I2C_Direction_Receiver);

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    /* While there is data to be read */
    while (NumByteToRead)
    {
        /* last byte no ACK */
        if (NumByteToRead == 1)
        {
            /* Disable Acknowledgement */
            I2C_AcknowledgeConfig(I2Cx, DISABLE);

            /* Send STOP Condition */
            I2C_GenerateSTOP(I2Cx, ENABLE);
        }

        /* Test on EV7 and clear it */
        if (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
        {
            /* Read a byte from the EEPROM */
            *pBuffer = I2C_ReceiveData(I2Cx);

            /* Point to the next location where the byte read will be saved */
            pBuffer++;

            /* Decrement the read bytes counter */
            NumByteToRead--;
        }
    }

    /* Enable Acknowledgement to be ready for another reception */
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
}


/**
  * @brief      write data of the pBuffer to the E2PROM
  * @param
  * @arg        pBuffer : buffer pointer
  * @arg        WriteAddr : address
  * @arg        NumByteToWrite : number byte to write
  * @retval
  */
void I2C_EE_BufferWrite(u8* pBuffer, u8 WriteAddr, u16 NumByteToWrite)
{
    u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;

    /* page address */
    Addr = WriteAddr % I2C_PageSize;
    /* offset in page */
    count = I2C_PageSize - Addr;
    NumOfPage = NumByteToWrite / I2C_PageSize;
    NumOfSingle = NumByteToWrite % I2C_PageSize;
    
    /* If WriteAddr is I2C_PageSize aligned  */
    if (Addr == 0)
    {
        /* If NumByteToWrite < I2C_PageSize */
        if (NumOfPage == 0)
        {
            I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            I2C_EE_WaitEepromStandbyState();
        }
        /* If NumByteToWrite > I2C_PageSize */
        else
        {
            while (NumOfPage--)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, I2C_PageSize);
                I2C_EE_WaitEepromStandbyState();
                WriteAddr += I2C_PageSize;
                pBuffer += I2C_PageSize;
            }

            if (NumOfSingle != 0)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                I2C_EE_WaitEepromStandbyState();
            }
        }
    }
    /* If WriteAddr is not I2C_PageSize aligned  */
    else
    {
        /* If NumByteToWrite < I2C_PageSize */
        if (NumOfPage == 0)
        {
            I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            I2C_EE_WaitEepromStandbyState();
        }
        /* If NumByteToWrite > I2C_PageSize */
        else
        {
            NumByteToWrite -= count;
            NumOfPage = NumByteToWrite / I2C_PageSize;
            NumOfSingle = NumByteToWrite % I2C_PageSize;

            if (count != 0)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, count);
                I2C_EE_WaitEepromStandbyState();
                WriteAddr += count;
                pBuffer += count;
            }

            while (NumOfPage--)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, I2C_PageSize);
                I2C_EE_WaitEepromStandbyState();
                WriteAddr += I2C_PageSize;
                pBuffer += I2C_PageSize;
            }
            if (NumOfSingle != 0)
            {
                I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                I2C_EE_WaitEepromStandbyState();
            }
        }
    }
}


/**
  * @brief   write 4 bytes to pca555 order : S /W command data0 data1 P
  * @param
  * @arg
  * @arg
  * @retval  null
  */
void I2C_PCA9555_Write(uint32_t val)
{
    int i;
    
    uint8_t* ptr = (uint8_t*)&val;

    /* Send STRAT condition */
    I2C_GenerateSTART(I2Cx, ENABLE);

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send PCA555 address for write */
    I2C_Send7bitAddress(I2Cx, ptr[0], I2C_Direction_Transmitter);

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    for(i=1;i<4;++i)
    {
        /* Send COMMAND data0 data1 */
        I2C_SendData(I2Cx, ptr[i]);
        /* Test on EV8 and clear it */
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }
    
    /* Send STOP condition */
    I2C_GenerateSTOP(I2Cx, ENABLE);
}

/*********************************************END OF FILE**********************/
