/**
* @file bsp_iic.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief use gpio emulation bus
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
#include "trace.h"


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

/* simulation iic bus clock */
#define SCL_1MHZ           (0)
#define SCL_400KHZ         (7)
#define SCL_100KHZ         (36)

/* fast mode scl duty */
#define SCL_DUTY_1_1       (0)
#define SCL_DUTY_2_1       (1)
#define SCL_DUTY_16_9      (2)

/* define IIC port&module */
#define SCL_DUTY            SCL_DUTY_1_1
#define RCC_GPIO_IIC        RCC_APB2Periph_GPIOB
#define GPIO_IIC            GPIOB
#define IIC_SCL             GPIO_Pin_10
#define IIC_SDA             GPIO_Pin_11

/* fast mode SCL Tlow/Thigh = 1:1*/
#if SCL_DUTY == SCL_DUTY_1_1
#define IIC_SCL_TLOW()     delay_nop(SCL_100KHZ)
#define IIC_SCL_THIGH()    delay_nop(SCL_100KHZ)
#elif SCL_DUTY == SCL_DUTY_2_1
#define IIC_SCL_TLOW()     delay_nop(SCL_100KHZ)
#define IIC_SCL_THIGH()    delay_nop(SCL_100KHZ)
#elif SCL_DUTY == SCL_DUTY_16_9
#define IIC_SCL_TLOW()     delay_nop(SCL_100KHZ)
#define IIC_SCL_THIGH()    delay_nop(SCL_100KHZ)
#else
#error "SCL duty error"
#endif

/* define function */
/* Master Transmitter */
#define IIC_SCL_1()          do {                           \
                                GPIO_IIC->BSRR = IIC_SCL;   \
                                IIC_SCL_TLOW();             \
                             } while(0)

#define IIC_SCL_0()          do {                           \
                                GPIO_IIC->BRR  = IIC_SCL;   \
                                IIC_SCL_THIGH();            \
                            } while(0)

#define IIC_SDA_1()         (GPIO_IIC->BSRR = IIC_SDA)
#define IIC_SDA_0()         (GPIO_IIC->BRR  = IIC_SDA)
#define IIC_SDA_READ()      ((GPIO_IIC->IDR & IIC_SDA) != 0)




void iic_sim_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /* PORT clock */
    RCC_APB2PeriphClockCmd(RCC_GPIO_IIC, ENABLE);

    /* I2C_SCL I2C_SDA */
    GPIO_InitStructure.GPIO_Pin = IIC_SCL | IIC_SDA;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIO_IIC, &GPIO_InitStructure);
    
    //force stop
    iic_stop();
}


void iic_start(void)
{
    IIC_SDA_1();
    IIC_SCL_1();

    IIC_SDA_0();
    __NOP();
    __NOP();
    __NOP();
    
    IIC_SCL_0();
}


void iic_send(uint8_t val)
{
    //MSB -> LSB
    for(int i=7; i>=0; --i)
    {
        (val & (1<<i)) ? IIC_SDA_1() : IIC_SDA_0();
        IIC_SCL_1();
        IIC_SCL_0();
    }
}


uint8_t iic_recv(void)
{
    uint8_t val = 0;
    //MSB <- LSB
    for(int i=7; i>=0; --i)
    {
        IIC_SCL_1();
        val |= (IIC_SDA_READ() ? (1<<i) : (0<<i));
        IIC_SCL_0();
    }

    return val;
}


uint8_t iic_waitack(void)
{
    uint8_t ret;
    
    //release sda
    IIC_SCL_1();
    //wait ack pending
    ret = IIC_SDA_READ();
    IIC_SCL_0();

    return (ret == 0);
}


void iic_ack(void)
{
    IIC_SDA_0();
    IIC_SCL_1();
    IIC_SCL_0();
    //release sda line
    IIC_SDA_1();
}


void iic_nack(void)
{
    //nack
    IIC_SDA_1();
    //clk
    IIC_SCL_1();
    IIC_SCL_0();
}


void iic_stop(void)
{
    IIC_SDA_0();
    __NOP();
    __NOP();
    __NOP();
    IIC_SCL_1();
    IIC_SDA_1();
}


////application for eeprom and pca9555
//uint8_t iic_device_online(uint8_t addr)
//{
//    uint8_t ack;
//    
//    iic_gpio_init();
//    
//    iic_start();
//    //addr(7b) + R/W(1b)
//    iic_send(addr | IIC_WR);
//    ack = iic_waitack();
//    iic_stop();

//    return (ack == 0);
//}

/*********************************************END OF FILE**********************/
