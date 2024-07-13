/****************************************************************
*名    称:FreeModbus移植实例
*作    者:温子祺
*创建日期:2021/08/02
*知 识 点:Modbus协议
*****************************************************************/
#include <stdio.h>
#include "stm32f4xx.h"
#include "led.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "tim.h"
#include "mb.h"


// 十路输入寄存器
#define REG_INPUT_SIZE  10
uint16_t REG_INPUT_BUF[REG_INPUT_SIZE];			// PLC的输入寄存器


// 十路保持寄存器
#define REG_HOLD_SIZE   10
uint16_t REG_HOLD_BUF[REG_HOLD_SIZE] = {1, 12, 13, 4, 5, 6, 7, 8, 9, 10};			// PLC的保持寄存器


// 十路线圈
#define REG_COILS_SIZE 10
uint8_t REG_COILS_BUF[REG_COILS_SIZE];			// PLC的线圈			


// 十路离散量
#define REG_DISC_SIZE  10
uint8_t REG_DISC_BUF[10];						// PLC的离散量 

int main(void)
{ 
	// 初始化LED	
	led_init();		

	// 设置中断优先级分组4
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	// 初始化定时器3
	tim3_init();
	
	// 串口1,波特率115200bps,开启接收中断
	usart1_init(115200);
	
	// RS485初始化
	rs485_init(9600);
	
	/*
		@brief		FreeModbus协议初始化(RemoteTerminalUnit，远程终端单元);地址：0x01; 端口：2; 波特率为9600bps，无校验
		@param		使用的是ModbusRTU
		@param		设备的id
		@param		ModbusRTU的端口号
		@param		ModbusRTU的波特率
		@param		ModbusRTU是否要效验
	*/
	eMBInit(MB_RTU, 0x01, 2, 9600, MB_PAR_NONE);
	
	/* 使能ModbusRTU */
	eMBEnable();	
	
	/* 上电打印 */
	printf("This is rs485 test\r\n");	
	
	
	while(1)
	{
		// 调用Modbus协议栈的轮询函数
		(void)eMBPoll();

	}
}

/*
	@brief 		处理04功能码(输入寄存器只能读)
				当Modbus主设备发起读取输入寄存器的请求时，这个函数就会被调用。

	@param		pucRegBuffer
				pucRegBuffer参数并不是输入寄存器本身，而是用来指向Modbus协议栈中用于存储输入寄存器数据的缓冲区的指针

	@param		usAddress
				通过usAddress参数确定了输入寄存器的起始地址。

	@param		usNRegs	
				根据usNRegs参数确定了要读取的寄存器数量。

	@param		eMode
				根据eMode参数的值，函数可以判断是进行读操作还是写操作

	// 读多个输入寄存器

	|-------------- Modbus 请求帧 ---------------|
	| 设备ID | 功能码 | 起始地址 | 寄存器数量 | CRC  |
	|  0x01  | 0x04  |  0x0010  |   0x0003   | CRC  |

	|-------- eMBRegInputCB 函数参数 ---------|
	| pucRegBuffer | usAddress | usNRegs  |
	|    数据      | 起始地址  | 寄存器数量 |
	|    N/A       |  0x0010   |  0x0003   |


*/
eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    USHORT usRegIndex = usAddress - 1; 	
	
	printf("eMBRegInputCB\r\n");
	
    // 非法检测
    if((usRegIndex + usNRegs) > REG_INPUT_SIZE)
    {
        return MB_ENOREG;
    }
	
    // 循环读取
    while( usNRegs > 0 )
    {
		// 将高位写入寄存器缓冲区
		*pucRegBuffer++ = ( unsigned char )( REG_INPUT_BUF[usRegIndex] >> 8 );
		
		// 将低位写入寄存器缓冲区
		*pucRegBuffer++ = ( unsigned char )( REG_INPUT_BUF[usRegIndex] & 0xFF );
	
        usRegIndex++;
        usNRegs--;
    }

    // 模拟输入寄存器被改变
    for(usRegIndex = 0; usRegIndex < REG_INPUT_SIZE; usRegIndex++)
    {
		// 模拟递增输入寄存器的值
        REG_INPUT_BUF[usRegIndex]++;
    }
	// 没有错误
    return MB_ENOERR;
}

/*
	@brief 		处理03, 06, 16功能码。
				当Modbus主设备发起读取或写入保持寄存器的请求时，这个函数就会被调用。

	@param		pucRegBuffer
				pucRegBuffer参数并不是保持寄存器本身，而是用来指向Modbus协议栈中用于存储保持寄存器数据的缓冲区的指针

	@param		usAddress
				通过usAddress参数确定了保持寄存器的起始地址。

	@param		usNRegs	
				根据usNRegs参数确定了要读取或写入的寄存器数量。

	@param		eMode
				根据eMode参数的值，函数可以判断是进行读操作还是写操作


	// 写单个保持寄存器
	|-------------- Modbus 请求帧 ---------------|
	| 设备ID  | 功能码 | 起始地址  | 数据   | CRC  |
	|  0x01   | 0x06   |  0x0010  | 0x1234 | CRC  |

	|-------- eMBRegHoldingCB 函数参数 ---------|
	| pucRegBuffer  | usAddress  | usNRegs | eMode |
	|    数据       | 起始地址    |   1     | 写入  |
	|    0x1234     |  0x0010    |         | MB_REG_WRITE |
	
	
	// 写多个保持寄存器
		
	|-------------- Modbus 请求帧 ---------------|
	| 设备ID | 功能码 | 起始地址 | 寄存器数量 | 字节计数 | 数据           | CRC  |
	|  0x01  | 0x10  |  0x0013  |   0x000A   |   0x14   | 0x123456...   | CRC  |

	|-------- eMBRegHoldingCB 函数参数 ---------|
	| pucRegBuffer | usAddress | usNRegs | eMode  |
	|    数据      | 起始地址  | 寄存器数量 | 写入   |
	|  0x123456... |  0x0013   |  0x000A   | MB_REG_WRITE |


*/
eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    USHORT usRegIndex = usAddress - 1;  
	printf("eMBRegHoldingCB\r\n");
	
    // 非法检测
    if((usRegIndex + usNRegs) > REG_HOLD_SIZE)
    {
		// 没有此寄存器的错误
        return MB_ENOREG;
    }
	
	printf("usAddress =%x\r\n", usAddress);					// 打印寄存器地址
	printf("usNRegs = %x\r\n", usNRegs);						// 打印寄存器数量	
	
	printf("pucRegBuffer[0]=%x\r\n", pucRegBuffer[0]);		// 打印寄存器缓冲区第一个字节	
	printf("pucRegBuffer[1]=%x\r\n", pucRegBuffer[1]);		// 打印寄存器缓冲区第二个字节
	
	// 上位机写寄存器请求，MCU读
    if(eMode == MB_REG_WRITE)
    {
		if(pucRegBuffer[1] ==0)
		{
			PFout(9)=1;
			PFout(10)=1;	
			PEout(13)=1;
			PEout(14)=1;			
		}	
		
		if(pucRegBuffer[1] ==1)
		{
			PFout(9)=0;
			PFout(10)=1;	
			PEout(13)=1;
			PEout(14)=1;
		}
		
		if(pucRegBuffer[1] ==2)
		{
			PFout(9)=0;
			PFout(10)=0;	
			PEout(13)=1;
			PEout(14)=1;
		}		
		
		if(pucRegBuffer[1] ==3)
		{
			PFout(9)=0;
			PFout(10)=0;	
			PEout(13)=0;
			PEout(14)=1;
		}		
				
		if(pucRegBuffer[1] ==4)
		{
			PFout(9)=0;
			PFout(10)=0;	
			PEout(13)=0;
			PEout(14)=0;
		}		
		
        while( usNRegs > 0 )
        {
			
			// 将寄存器缓冲区的值写入保持寄存器
			REG_HOLD_BUF[usRegIndex] = (pucRegBuffer[0] << 8) | pucRegBuffer[1];
			pucRegBuffer += 2;
            usRegIndex++;
            usNRegs--;
        }
    }
	
	// 读保持寄存器
    else
    {
        while( usNRegs > 0 )
        {
			// 将保持寄存器的高位写入寄存器缓冲区
            *pucRegBuffer++ = ( unsigned char )( REG_HOLD_BUF[usRegIndex] >> 8 );
			
			// 将保持寄存器的低位写入寄存器缓冲区
            *pucRegBuffer++ = ( unsigned char )( REG_HOLD_BUF[usRegIndex] & 0xFF );
            usRegIndex++;
            usNRegs--;
        }
    }
	// 没有错误
    return MB_ENOERR;
}

/*
	@brief 		处理01, 05, 15功能码。
				当Modbus主设备发起读取或写入线圈的请求时，这个函数就会被调用。

	@param		pucRegBuffer
					pucRegBuffer存储主机发送来的帧协议。存储线圈的状态

	@param		usAddress
				通过usAddress参数确定了线圈的起始地址。

	@param		usNRegs	
				根据usNRegs参数确定了要读取或写入的线圈数量。

	@param		eMode
				根据eMode参数的值，函数可以判断是进行读操作还是写操作

	// 写单个线圈
		|-------------- Modbus 请求帧 ---------------|
		| 设备ID | 功能码 | 起始地址 | 数据  | CRC  |
		|  0x01  | 0x05  |  0x0000  | 0xFF00 | CRC  |

		|-------- eMBRegCoilsCB 函数参数 ---------|
		| pucRegBuffer | usAddress | usNCoils | eMode |
		|    数据      | 起始地址  |   1      | 写入  |
		|   0xFF00     |  0x0000   |          | MB_REG_WRITE |
		
	// 写多个线圈
		
	|-------------- Modbus 请求帧 ---------------|
	| 设备ID | 功能码 | 起始地址 | 线圈数量 | 字节计数 | 线圈值  | CRC  |
	|  0x01  | 0x0F  |  0x0013  |  0x000A  |   0x02   | 0xCD01 | CRC  |

	|-------- eMBRegCoilsCB 函数参数 ---------|
	| pucRegBuffer | usAddress | usNCoils | eMode  |
	|    线圈值    | 起始地址  | 线圈数量 | 写入   |
	|    0xCD01    |  0x0013   |  0x000A  | MB_REG_WRITE |


	// 读单个线圈
		
	|-------------- Modbus 请求帧 ---------------|
	| 设备ID | 功能码 | 起始地址 | 线圈数量 | CRC  |
	|  0x01  | 0x01  |  0x0013  |  0x0007  | CRC  |

	|-------- eMBRegCoilsCB 函数参数 ---------|
	| pucRegBuffer | usAddress | usNCoils | eMode |
	|    数据      | 起始地址  | 线圈数量 | 读取  |
	|              |  0x0013   |  0x0007  | MB_REG_READ |

*/
eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
    USHORT usRegIndex   = usAddress - 1;
    USHORT usCoilGroups = ((usNCoils - 1) / 8 + 1);
    UCHAR  ucStatus     = 0;
    UCHAR  ucBits       = 0;
    UCHAR  ucDisp       = 0;
	printf("eMBRegCoilsCB\r\n");					// 打印调试信息					
	
	printf("usAddress=%x\r\n",usAddress);			// 打印寄存器地址
	printf("usNCoils=%x\r\n",usNCoils);				// 打印线圈数量	
	printf("eMode=%x\r\n",eMode);					// 打印寄存器模式			
    // 非法检测
    if((usRegIndex + usNCoils) > REG_COILS_SIZE)
    {
		// 没有此寄存器的错误
        return MB_ENOREG;
    }
	

    // 写线圈
    if(eMode == MB_REG_WRITE)
    {
        while(usCoilGroups--)
        {
            ucStatus = *pucRegBuffer++;
            ucBits   = 8;
            while((usNCoils--) != 0 && (ucBits--) != 0)
            {
                REG_COILS_BUF[usRegIndex++] = ucStatus & 0X01;		// 将线圈状态写入线圈缓冲区
                ucStatus >>= 1;
            }
        }
    }

    // 读线圈
    else
    {
        while(usCoilGroups--)
        {
            ucDisp = 0;
            ucBits = 8;
            while((usNCoils--) != 0 && (ucBits--) != 0)
            {
                ucStatus |= (REG_COILS_BUF[usRegIndex++] << (ucDisp++));		// 从线圈缓冲区读取线圈状态
            }
            *pucRegBuffer++ = ucStatus;
        }
    }
	
	// 没有错误
    return MB_ENOERR;
}


/*
	@brief 		处理02功能码。
				当Modbus主设备发起读离散输入的请求时，这个函数就会被调用。

	@param		pucRegBuffer
					pucRegBuffer存储主机发送来的帧协议。存储线圈的状态

	@param		usAddress
					通过usAddress参数确定了线圈的起始地址。

	@param		usNDiscrete
					离散输入数量

	|-------------- Modbus 请求帧 ---------------|
	| 设备ID | 功能码 | 起始地址 | 离散输入数量 | CRC  |
	|  0x01  | 0x02  |  0x0010  |    0x0005    | CRC  |

	|-------- eMBRegDiscreteCB 函数参数 ---------|
	| pucRegBuffer | usAddress | usNDiscrete  |
	|    数据      | 起始地址  | 离散输入数量 |
	|    N/A       |  0x0010   |    0x0005    |

*/
eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    USHORT usRegIndex   = usAddress - 1;
    USHORT usCoilGroups = ((usNDiscrete - 1) / 8 + 1);
    UCHAR  ucStatus     = 0;
    UCHAR  ucBits       = 0;
    UCHAR  ucDisp       = 0;
	printf("eMBRegDiscreteCB\r\n");						// 打印调试信息
    // 非法检测
    if((usRegIndex + usNDiscrete) > REG_DISC_SIZE)
    {
		// 没有此寄存器的错误
        return MB_ENOREG;
    }

	// 读离散输入
	while(usCoilGroups--)
	{
		ucDisp = 0;
		ucBits = 8;
		while((usNDiscrete--) != 0 && (ucBits--) != 0)
		{
			if(REG_DISC_BUF[usRegIndex])
			{
				ucStatus |= (1 << ucDisp);
			}
			ucDisp++;
		}
		*pucRegBuffer++ = ucStatus;
	}

    // 模拟改变
    for(usRegIndex = 0; usRegIndex < REG_DISC_SIZE; usRegIndex++)
    {
        REG_DISC_BUF[usRegIndex] = !REG_DISC_BUF[usRegIndex];			// 模拟离散量改变
    }
	
	// 没有错误
    return MB_ENOERR;							
}

