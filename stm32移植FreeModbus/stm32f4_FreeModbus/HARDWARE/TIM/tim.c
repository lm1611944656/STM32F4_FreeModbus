#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "tim.h"
#include "mb.h"
#include <stdio.h>

void tim3_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;	
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	//使能定时器3的硬件时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 4000-1;		//t3.5字符超时定时器的时钟
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;	//预分频值
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;		//定时器3的中断号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级 0x2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//响应优先级 0x2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//NVIC使能定时器3中断请求通道
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM3, DISABLE);	
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	TIM_ITConfig(TIM3,TIM_IT_Update , DISABLE);
}

extern void prvvTIMERExpiredISR( void );

void TIM3_IRQHandler(void)
{

	//检测标志位
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) == SET)
	{
		
		prvvTIMERExpiredISR();					// 通知modbus3.5个字符等待时间到
			
		//清空标志位
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	
	}
}
