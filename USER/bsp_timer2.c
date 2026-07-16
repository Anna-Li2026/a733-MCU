/**
* @file isr.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief
*
* @details timer2
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

#include "stm32f10x.h"
#include "bsp_timer2.h"
#include "dhcp.h"
#include "bsp_inter_xfer.h"
#include "trace.h"


/**
 * @brief timer initialization
 * TIM period / Auto Reload Register(ARR) = 1000, TIM_Prescaler = 71
 * Interrupt cycle : 1/(72MHZ/prescaler) us * 1000 = 1ms
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> occor interrupt then TIMxCNT set zero
 */
void Timer2_Init_Config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);                            //timer2 clock

    /* Interrupt cycle : 1/(TIM2CLK/(TIM_prescaler+1)) * TIM_Period */
    TIM_TimeBaseStructure.TIM_Period = 4000;                                        //Period 0.25 ms *4000 => 1s
    TIM_TimeBaseStructure.TIM_Prescaler = 18 * 1000 - 1;                            //Prescaler 4.5MHz/18*1000 => 4.5KHz/18 => 0.25ms
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV4;                         //TIM2CLK 72/16MHz => 4.5MHz
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);                                       //timer2 interrupt
    TIM_Cmd(TIM2, ENABLE);

    /* timer NVIC */
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}




/**
 * @brief timer2 ISR interrupt period 1 second
 */
void TIM2_IRQHandler(void)
{
    //TRIG_LOW(TRIG7);

    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
			  
        DHCP_time_handler();
			  xfer_time_handler();
    }

    //TRIG_HIGH(TRIG7);
}

