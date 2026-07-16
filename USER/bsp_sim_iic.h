#ifndef __BSP_SIM_IIC_H
#define __BSP_SIM_IIC_H
#include "stm32f10x.h"

//access bit
#define IIC_WR      (0<<0)
#define IIC_RD      (1<<0)

void iic_sim_init(void);
void iic_start(void);
void iic_stop(void);
void iic_send(uint8_t val);
uint8_t iic_recv(void);
uint8_t iic_waitack(void);
void iic_ack(void);
void iic_nack(void);

#endif
