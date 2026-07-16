#ifndef _SysTick_H
#define _SysTick_H

void SysTick_Init(int SYSCLK);
void delay_nop(int n);
void delay_us(int us);
void delay_ms(int ms);
void measure_start(void);
int measure_ms(void);

#endif
