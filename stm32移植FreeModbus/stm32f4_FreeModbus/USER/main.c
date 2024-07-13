/****************************************************************
*��    ��:FreeModbus��ֲʵ��
*��    ��:������
*��������:2021/08/02
*֪ ʶ ��:ModbusЭ��
*****************************************************************/
#include <stdio.h>
#include "stm32f4xx.h"
#include "led.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "tim.h"
#include "mb.h"


// ʮ·����Ĵ���
#define REG_INPUT_SIZE  10
uint16_t REG_INPUT_BUF[REG_INPUT_SIZE];			// PLC������Ĵ���


// ʮ·���ּĴ���
#define REG_HOLD_SIZE   10
uint16_t REG_HOLD_BUF[REG_HOLD_SIZE] = {1, 12, 13, 4, 5, 6, 7, 8, 9, 10};			// PLC�ı��ּĴ���


// ʮ·��Ȧ
#define REG_COILS_SIZE 10
uint8_t REG_COILS_BUF[REG_COILS_SIZE];			// PLC����Ȧ			


// ʮ·��ɢ��
#define REG_DISC_SIZE  10
uint8_t REG_DISC_BUF[10];						// PLC����ɢ�� 

int main(void)
{ 
	// ��ʼ��LED	
	led_init();		

	// �����ж����ȼ�����4
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	// ��ʼ����ʱ��3
	tim3_init();
	
	// ����1,������115200bps,���������ж�
	usart1_init(115200);
	
	// RS485��ʼ��
	rs485_init(9600);
	
	/*
		@brief		FreeModbusЭ���ʼ��(RemoteTerminalUnit��Զ���ն˵�Ԫ);��ַ��0x01; �˿ڣ�2; ������Ϊ9600bps����У��
		@param		ʹ�õ���ModbusRTU
		@param		�豸��id
		@param		ModbusRTU�Ķ˿ں�
		@param		ModbusRTU�Ĳ�����
		@param		ModbusRTU�Ƿ�ҪЧ��
	*/
	eMBInit(MB_RTU, 0x01, 2, 9600, MB_PAR_NONE);
	
	/* ʹ��ModbusRTU */
	eMBEnable();	
	
	/* �ϵ��ӡ */
	printf("This is rs485 test\r\n");	
	
	
	while(1)
	{
		// ����ModbusЭ��ջ����ѯ����
		(void)eMBPoll();

	}
}

/*
	@brief 		����04������(����Ĵ���ֻ�ܶ�)
				��Modbus���豸�����ȡ����Ĵ���������ʱ����������ͻᱻ���á�

	@param		pucRegBuffer
				pucRegBuffer��������������Ĵ���������������ָ��ModbusЭ��ջ�����ڴ洢����Ĵ������ݵĻ�������ָ��

	@param		usAddress
				ͨ��usAddress����ȷ��������Ĵ�������ʼ��ַ��

	@param		usNRegs	
				����usNRegs����ȷ����Ҫ��ȡ�ļĴ���������

	@param		eMode
				����eMode������ֵ�����������ж��ǽ��ж���������д����

	// ���������Ĵ���

	|-------------- Modbus ����֡ ---------------|
	| �豸ID | ������ | ��ʼ��ַ | �Ĵ������� | CRC  |
	|  0x01  | 0x04  |  0x0010  |   0x0003   | CRC  |

	|-------- eMBRegInputCB �������� ---------|
	| pucRegBuffer | usAddress | usNRegs  |
	|    ����      | ��ʼ��ַ  | �Ĵ������� |
	|    N/A       |  0x0010   |  0x0003   |


*/
eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    USHORT usRegIndex = usAddress - 1; 	
	
	printf("eMBRegInputCB\r\n");
	
    // �Ƿ����
    if((usRegIndex + usNRegs) > REG_INPUT_SIZE)
    {
        return MB_ENOREG;
    }
	
    // ѭ����ȡ
    while( usNRegs > 0 )
    {
		// ����λд��Ĵ���������
		*pucRegBuffer++ = ( unsigned char )( REG_INPUT_BUF[usRegIndex] >> 8 );
		
		// ����λд��Ĵ���������
		*pucRegBuffer++ = ( unsigned char )( REG_INPUT_BUF[usRegIndex] & 0xFF );
	
        usRegIndex++;
        usNRegs--;
    }

    // ģ������Ĵ������ı�
    for(usRegIndex = 0; usRegIndex < REG_INPUT_SIZE; usRegIndex++)
    {
		// ģ���������Ĵ�����ֵ
        REG_INPUT_BUF[usRegIndex]++;
    }
	// û�д���
    return MB_ENOERR;
}

/*
	@brief 		����03, 06, 16�����롣
				��Modbus���豸�����ȡ��д�뱣�ּĴ���������ʱ����������ͻᱻ���á�

	@param		pucRegBuffer
				pucRegBuffer���������Ǳ��ּĴ���������������ָ��ModbusЭ��ջ�����ڴ洢���ּĴ������ݵĻ�������ָ��

	@param		usAddress
				ͨ��usAddress����ȷ���˱��ּĴ�������ʼ��ַ��

	@param		usNRegs	
				����usNRegs����ȷ����Ҫ��ȡ��д��ļĴ���������

	@param		eMode
				����eMode������ֵ�����������ж��ǽ��ж���������д����


	// д�������ּĴ���
	|-------------- Modbus ����֡ ---------------|
	| �豸ID  | ������ | ��ʼ��ַ  | ����   | CRC  |
	|  0x01   | 0x06   |  0x0010  | 0x1234 | CRC  |

	|-------- eMBRegHoldingCB �������� ---------|
	| pucRegBuffer  | usAddress  | usNRegs | eMode |
	|    ����       | ��ʼ��ַ    |   1     | д��  |
	|    0x1234     |  0x0010    |         | MB_REG_WRITE |
	
	
	// д������ּĴ���
		
	|-------------- Modbus ����֡ ---------------|
	| �豸ID | ������ | ��ʼ��ַ | �Ĵ������� | �ֽڼ��� | ����           | CRC  |
	|  0x01  | 0x10  |  0x0013  |   0x000A   |   0x14   | 0x123456...   | CRC  |

	|-------- eMBRegHoldingCB �������� ---------|
	| pucRegBuffer | usAddress | usNRegs | eMode  |
	|    ����      | ��ʼ��ַ  | �Ĵ������� | д��   |
	|  0x123456... |  0x0013   |  0x000A   | MB_REG_WRITE |


*/
eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    USHORT usRegIndex = usAddress - 1;  
	printf("eMBRegHoldingCB\r\n");
	
    // �Ƿ����
    if((usRegIndex + usNRegs) > REG_HOLD_SIZE)
    {
		// û�д˼Ĵ����Ĵ���
        return MB_ENOREG;
    }
	
	printf("usAddress =%x\r\n", usAddress);					// ��ӡ�Ĵ�����ַ
	printf("usNRegs = %x\r\n", usNRegs);						// ��ӡ�Ĵ�������	
	
	printf("pucRegBuffer[0]=%x\r\n", pucRegBuffer[0]);		// ��ӡ�Ĵ�����������һ���ֽ�	
	printf("pucRegBuffer[1]=%x\r\n", pucRegBuffer[1]);		// ��ӡ�Ĵ����������ڶ����ֽ�
	
	// ��λ��д�Ĵ�������MCU��
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
			
			// ���Ĵ�����������ֵд�뱣�ּĴ���
			REG_HOLD_BUF[usRegIndex] = (pucRegBuffer[0] << 8) | pucRegBuffer[1];
			pucRegBuffer += 2;
            usRegIndex++;
            usNRegs--;
        }
    }
	
	// �����ּĴ���
    else
    {
        while( usNRegs > 0 )
        {
			// �����ּĴ����ĸ�λд��Ĵ���������
            *pucRegBuffer++ = ( unsigned char )( REG_HOLD_BUF[usRegIndex] >> 8 );
			
			// �����ּĴ����ĵ�λд��Ĵ���������
            *pucRegBuffer++ = ( unsigned char )( REG_HOLD_BUF[usRegIndex] & 0xFF );
            usRegIndex++;
            usNRegs--;
        }
    }
	// û�д���
    return MB_ENOERR;
}

/*
	@brief 		����01, 05, 15�����롣
				��Modbus���豸�����ȡ��д����Ȧ������ʱ����������ͻᱻ���á�

	@param		pucRegBuffer
					pucRegBuffer�洢������������֡Э�顣�洢��Ȧ��״̬

	@param		usAddress
				ͨ��usAddress����ȷ������Ȧ����ʼ��ַ��

	@param		usNRegs	
				����usNRegs����ȷ����Ҫ��ȡ��д�����Ȧ������

	@param		eMode
				����eMode������ֵ�����������ж��ǽ��ж���������д����

	// д������Ȧ
		|-------------- Modbus ����֡ ---------------|
		| �豸ID | ������ | ��ʼ��ַ | ����  | CRC  |
		|  0x01  | 0x05  |  0x0000  | 0xFF00 | CRC  |

		|-------- eMBRegCoilsCB �������� ---------|
		| pucRegBuffer | usAddress | usNCoils | eMode |
		|    ����      | ��ʼ��ַ  |   1      | д��  |
		|   0xFF00     |  0x0000   |          | MB_REG_WRITE |
		
	// д�����Ȧ
		
	|-------------- Modbus ����֡ ---------------|
	| �豸ID | ������ | ��ʼ��ַ | ��Ȧ���� | �ֽڼ��� | ��Ȧֵ  | CRC  |
	|  0x01  | 0x0F  |  0x0013  |  0x000A  |   0x02   | 0xCD01 | CRC  |

	|-------- eMBRegCoilsCB �������� ---------|
	| pucRegBuffer | usAddress | usNCoils | eMode  |
	|    ��Ȧֵ    | ��ʼ��ַ  | ��Ȧ���� | д��   |
	|    0xCD01    |  0x0013   |  0x000A  | MB_REG_WRITE |


	// ��������Ȧ
		
	|-------------- Modbus ����֡ ---------------|
	| �豸ID | ������ | ��ʼ��ַ | ��Ȧ���� | CRC  |
	|  0x01  | 0x01  |  0x0013  |  0x0007  | CRC  |

	|-------- eMBRegCoilsCB �������� ---------|
	| pucRegBuffer | usAddress | usNCoils | eMode |
	|    ����      | ��ʼ��ַ  | ��Ȧ���� | ��ȡ  |
	|              |  0x0013   |  0x0007  | MB_REG_READ |

*/
eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
    USHORT usRegIndex   = usAddress - 1;
    USHORT usCoilGroups = ((usNCoils - 1) / 8 + 1);
    UCHAR  ucStatus     = 0;
    UCHAR  ucBits       = 0;
    UCHAR  ucDisp       = 0;
	printf("eMBRegCoilsCB\r\n");					// ��ӡ������Ϣ					
	
	printf("usAddress=%x\r\n",usAddress);			// ��ӡ�Ĵ�����ַ
	printf("usNCoils=%x\r\n",usNCoils);				// ��ӡ��Ȧ����	
	printf("eMode=%x\r\n",eMode);					// ��ӡ�Ĵ���ģʽ			
    // �Ƿ����
    if((usRegIndex + usNCoils) > REG_COILS_SIZE)
    {
		// û�д˼Ĵ����Ĵ���
        return MB_ENOREG;
    }
	

    // д��Ȧ
    if(eMode == MB_REG_WRITE)
    {
        while(usCoilGroups--)
        {
            ucStatus = *pucRegBuffer++;
            ucBits   = 8;
            while((usNCoils--) != 0 && (ucBits--) != 0)
            {
                REG_COILS_BUF[usRegIndex++] = ucStatus & 0X01;		// ����Ȧ״̬д����Ȧ������
                ucStatus >>= 1;
            }
        }
    }

    // ����Ȧ
    else
    {
        while(usCoilGroups--)
        {
            ucDisp = 0;
            ucBits = 8;
            while((usNCoils--) != 0 && (ucBits--) != 0)
            {
                ucStatus |= (REG_COILS_BUF[usRegIndex++] << (ucDisp++));		// ����Ȧ��������ȡ��Ȧ״̬
            }
            *pucRegBuffer++ = ucStatus;
        }
    }
	
	// û�д���
    return MB_ENOERR;
}


/*
	@brief 		����02�����롣
				��Modbus���豸�������ɢ���������ʱ����������ͻᱻ���á�

	@param		pucRegBuffer
					pucRegBuffer�洢������������֡Э�顣�洢��Ȧ��״̬

	@param		usAddress
					ͨ��usAddress����ȷ������Ȧ����ʼ��ַ��

	@param		usNDiscrete
					��ɢ��������

	|-------------- Modbus ����֡ ---------------|
	| �豸ID | ������ | ��ʼ��ַ | ��ɢ�������� | CRC  |
	|  0x01  | 0x02  |  0x0010  |    0x0005    | CRC  |

	|-------- eMBRegDiscreteCB �������� ---------|
	| pucRegBuffer | usAddress | usNDiscrete  |
	|    ����      | ��ʼ��ַ  | ��ɢ�������� |
	|    N/A       |  0x0010   |    0x0005    |

*/
eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    USHORT usRegIndex   = usAddress - 1;
    USHORT usCoilGroups = ((usNDiscrete - 1) / 8 + 1);
    UCHAR  ucStatus     = 0;
    UCHAR  ucBits       = 0;
    UCHAR  ucDisp       = 0;
	printf("eMBRegDiscreteCB\r\n");						// ��ӡ������Ϣ
    // �Ƿ����
    if((usRegIndex + usNDiscrete) > REG_DISC_SIZE)
    {
		// û�д˼Ĵ����Ĵ���
        return MB_ENOREG;
    }

	// ����ɢ����
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

    // ģ��ı�
    for(usRegIndex = 0; usRegIndex < REG_DISC_SIZE; usRegIndex++)
    {
        REG_DISC_BUF[usRegIndex] = !REG_DISC_BUF[usRegIndex];			// ģ����ɢ���ı�
    }
	
	// û�д���
    return MB_ENOERR;							
}

