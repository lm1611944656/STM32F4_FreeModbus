#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "mb.h"
#include "mbport.h"
#include <stdio.h>

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) 
{

	USART_SendData(USART1,ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);	
	
	return(ch);
}

void usart1_init(uint32_t baud)
{

	GPIO_InitTypeDef  	GPIO_InitStructure;
	USART_InitTypeDef 	USART_InitStructure;
	NVIC_InitTypeDef 	NVIC_InitStructure;			
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 							//使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);							//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); 						//GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); 						//GPIOA10复用为USART1
	
	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; 						//GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;									//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;								//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); 											//初始化PA9，PA10

	//USART1 初始化设置
	USART_InitStructure.USART_BaudRate = baud;										//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式
	USART_Init(USART1, &USART_InitStructure); 										//初始化串口1
	
	USART_Cmd(USART1, ENABLE);  													//使能串口1 
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);									//开启相关中断

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;								//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;							//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;								//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;									//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);													//根据指定的参数初始化VIC寄存器
}


void usart1_send_bytes(uint8_t *pbuf,uint32_t len)
{
	uint32_t i =0;
	
	for(i=0; i<len; i++)
	{
		//发送数据
		USART_SendData(USART1,*pbuf++);
		
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);		
	}	
}

//初始化IO 串口2
//baud:波特率	  
void rs485_init(uint32_t baud)
{  	 
	GPIO_InitTypeDef  	GPIO_InitStructure;
	USART_InitTypeDef 	USART_InitStructure;
	NVIC_InitTypeDef 	NVIC_InitStructure;	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 				//使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);				//使能USART2时钟
	
	//串口2引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); 			//GPIOA2复用为USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); 			//GPIOA3复用为USART2
	
	//USART2    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; 				//GPIOA2与GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;						//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;					//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 						//推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 						//上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); 								//初始化PA2，PA3
	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE); 				//使能GPIOA时钟
	
	//PG8推挽输出，485模式控制  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 							//GPIOG8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;						//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;					//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 						//推挽输出
	GPIO_InitStructure.GPIO_PuPd  =  GPIO_PuPd_NOPULL; 						//上拉
	GPIO_Init(GPIOG,&GPIO_InitStructure); 								//初始化PG8
	

	//USART2 初始化设置
	USART_InitStructure.USART_BaudRate = baud;							//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;			//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;				//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;					//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		//收发模式
	USART_Init(USART2, &USART_InitStructure); 							//初始化串口2
	
	USART_Cmd(USART2, ENABLE);  										//使能串口 2
	USART_ClearFlag(USART2, USART_FLAG_TC);
	

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);						//开启接收中断

	//Usart2 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;				//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;					//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;						//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);										//根据指定的参数初始化NVIC寄存器、
	RS485_TX_EN=0;														//默认为接收模式	
}


void rs485_send_bytes(uint8_t *pbuf,uint32_t len)
{
	uint32_t i;
	
	delay_ms(1);		
	RS485_TX_EN=1;												//设置为发送模式
	delay_ms(1);
	for(i=0; i<len; i++)										//循环发送数据
	{
		USART_SendData(USART2,pbuf[i]); 						//发送数据
		while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);	
	}	 
	delay_ms(1);	
	RS485_TX_EN=0;												//设置为接收模式	
	delay_ms(1);		
}

void USART1_IRQHandler(void)                				//串口1中断服务程序
{
	uint8_t d;

	if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)  	//接收中断
	{
		//接收数据
		d = USART_ReceiveData(USART1);	
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	} 
} 

extern void prvvUARTTxReadyISR( void );
extern void prvvUARTRxISR( void );

void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)	//接收到数据
	{	 
		//数据读取prvvUARTRxISR函数中执行
		prvvUARTRxISR();		
		
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);		
	}  		

	if(USART_GetITStatus(USART2, USART_IT_TXE) == SET)	//发送完成
	{	 
					
		prvvUARTTxReadyISR();	
		USART_ClearITPendingBit(USART2, USART_IT_TXE);		
	}  
} 

