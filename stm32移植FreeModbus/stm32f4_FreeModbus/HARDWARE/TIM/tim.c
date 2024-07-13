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
	
	//ʹ�ܶ�ʱ��3��Ӳ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 4000-1;		//t3.5�ַ���ʱ��ʱ����ʱ��
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;	//Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//���ϼ���
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;		//��ʱ��3���жϺ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ� 0x2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//��Ӧ���ȼ� 0x2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//NVICʹ�ܶ�ʱ��3�ж�����ͨ��
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM3, DISABLE);	
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	TIM_ITConfig(TIM3,TIM_IT_Update , DISABLE);
}

extern void prvvTIMERExpiredISR( void );

void TIM3_IRQHandler(void)
{

	//����־λ
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) == SET)
	{
		
		prvvTIMERExpiredISR();					// ֪ͨmodbus3.5���ַ��ȴ�ʱ�䵽
			
		//��ձ�־λ
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	
	}
}
