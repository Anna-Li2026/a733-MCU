#include "stm32f10x.h"
#include "SysTick.h"

static int SysTickCLK = 0;

/**
 * @brief init system tick, the SYSTICK is AHB clock/8
 * @param SYSCLK MHz, default is 72MHZ
 * @retval
 */
void SysTick_Init(int SYSCLK)
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);    //AHB/8
    SysTickCLK = SYSCLK / 8;                                 //MHz
    SysTick->CTRL = 0;
}



/**
 * @brief 
 * @param count of us
 * @retval
 */
void delay_nop(int n)
{
    do
    {
        __NOP();
    }while(n--);
}


/**
 * @brief a count per T = 1/SysTickCLK
 * @param count of us
 * @retval
 */
void delay_us(int us)
{
    SysTick->LOAD = us * SysTickCLK - 1;                        //one count elapsed 1/SysTickCLK us, so us*SysTickCLK / SysTickCLK
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
}


/**
 * @brief delay about ms, max delay 1864ms(0xffffff*8*1000/SYSCLK)
 * @param
 * @retval
 */
void delay_ms(int ms)
{
    SysTick->LOAD = ms * 1000 * SysTickCLK - 1;                //1ms -> 1000us
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
}


/**
 * @brief start measure time
 * @param
 * @retval
 */
void measure_start(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;                    //stop it

    SysTick->LOAD = 0xffffff;
    SysTick->VAL = 0xffffff;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    __NOP();
}


/**
 * @brief stop measure time, max 1864ms
 * @param [in] measure time unit US
 * @retval program run time
 */
int measure_ms(void)
{
    //SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    return (0xffffff - (SysTick->VAL & 0xffffff)) / (SysTickCLK * 1000);
}
