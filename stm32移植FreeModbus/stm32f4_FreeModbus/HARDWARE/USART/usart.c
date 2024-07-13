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
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 							//ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);							//ʹ��USART1ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); 						//GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); 						//GPIOA10����ΪUSART1
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; 						//GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;									//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;								//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//����
	GPIO_Init(GPIOA,&GPIO_InitStructure); 											//��ʼ��PA9��PA10

	//USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = baud;										//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//�շ�ģʽ
	USART_Init(USART1, &USART_InitStructure); 										//��ʼ������1
	
	USART_Cmd(USART1, ENABLE);  													//ʹ�ܴ���1 
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);									//��������ж�

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;								//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;							//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;								//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;									//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);													//����ָ���Ĳ�����ʼ��VIC�Ĵ���
}


void usart1_send_bytes(uint8_t *pbuf,uint32_t len)
{
	uint32_t i =0;
	
	for(i=0; i<len; i++)
	{
		//��������
		USART_SendData(USART1,*pbuf++);
		
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);		
	}	
}

//��ʼ��IO ����2
//baud:������	  
void rs485_init(uint32_t baud)
{  	 
	GPIO_InitTypeDef  	GPIO_InitStructure;
	USART_InitTypeDef 	USART_InitStructure;
	NVIC_InitTypeDef 	NVIC_InitStructure;	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 				//ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);				//ʹ��USART2ʱ��
	
	//����2���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); 			//GPIOA2����ΪUSART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); 			//GPIOA3����ΪUSART2
	
	//USART2    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; 				//GPIOA2��GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;						//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;					//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 						//���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 						//����
	GPIO_Init(GPIOA,&GPIO_InitStructure); 								//��ʼ��PA2��PA3
	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE); 				//ʹ��GPIOAʱ��
	
	//PG8���������485ģʽ����  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 							//GPIOG8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;						//���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;					//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 						//�������
	GPIO_InitStructure.GPIO_PuPd  =  GPIO_PuPd_NOPULL; 						//����
	GPIO_Init(GPIOG,&GPIO_InitStructure); 								//��ʼ��PG8
	

	//USART2 ��ʼ������
	USART_InitStructure.USART_BaudRate = baud;							//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;			//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;				//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;					//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); 							//��ʼ������2
	
	USART_Cmd(USART2, ENABLE);  										//ʹ�ܴ��� 2
	USART_ClearFlag(USART2, USART_FLAG_TC);
	

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);						//���������ж�

	//Usart2 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;				//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;					//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;						//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);										//����ָ���Ĳ�����ʼ��NVIC�Ĵ�����
	RS485_TX_EN=0;														//Ĭ��Ϊ����ģʽ	
}


void rs485_send_bytes(uint8_t *pbuf,uint32_t len)
{
	uint32_t i;
	
	delay_ms(1);		
	RS485_TX_EN=1;												//����Ϊ����ģʽ
	delay_ms(1);
	for(i=0; i<len; i++)										//ѭ����������
	{
		USART_SendData(USART2,pbuf[i]); 						//��������
		while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);	
	}	 
	delay_ms(1);	
	RS485_TX_EN=0;												//����Ϊ����ģʽ	
	delay_ms(1);		
}

void USART1_IRQHandler(void)                				//����1�жϷ������
{
	uint8_t d;

	if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)  	//�����ж�
	{
		//��������
		d = USART_ReceiveData(USART1);	
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	} 
} 

extern void prvvUARTTxReadyISR( void );
extern void prvvUARTRxISR( void );

void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)	//���յ�����
	{	 
		//���ݶ�ȡprvvUARTRxISR������ִ��
		prvvUARTRxISR();		
		
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);		
	}  		

	if(USART_GetITStatus(USART2, USART_IT_TXE) == SET)	//�������
	{	 
					
		prvvUARTTxReadyISR();	
		USART_ClearITPendingBit(USART2, USART_IT_TXE);		
	}  
} 

