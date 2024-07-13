#include "delay.h"

void delay_us(uint32_t nus)
{		
	uint32_t temp;	    	 
	SysTick->LOAD =SystemCoreClock/8/1000000*nus; 	//时间加载	  		 
	SysTick->VAL  =0x00;        					//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ; 		//使能滴答定时器开始倒数 	 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));			//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; 		//关闭计数器
	SysTick->VAL =0X00;       						//清空计数器 
}

void delay_ms(uint16_t nms)
{	 		  	  
	uint32_t temp;		   
	SysTick->LOAD=SystemCoreClock/8/1000*nms;		//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;           					//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;    	//能滴答定时器开始倒数 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));			//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;    	//关闭计数器
	SysTick->VAL =0X00;     		  				//清空计数器	  	    
} 



































