注意PLC的地址是从1开始，而MCU的地址是从0开始；

```c
eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
	// PLC的地址先减法从0地址开始
    USHORT usRegIndex = usAddress - 1;  
	printf("eMBRegHoldingCB\r\n");
	
    // 非法检测
    if((usRegIndex + usNRegs) > REG_HOLD_SIZE)
    {
		// 没有此寄存器的错误
        return MB_ENOREG;
    }
	
	printf("usAddress =%x\r\n", usAddress);					// 打印寄存器地址1
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
			// 取出保持寄存器的高8位写入寄存器缓冲区
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
```



所以在操作PLC传递来的地址，需要将地址减法操作(目的是从0开始)

- 示例1：

主机发送

> 000004-Tx:01 03 **00 00 00 0A** C5 CD
> 000005-Rx:01 03 14 00 01 00 0C 00 0D 00 04 00 05 00 06 00 07 00 08 00 09 00 0A 65 71



串口打印信息：

>eMBRegHoldingCB
>usAddress =1
>usNRegs = a
>pucRegBuffer[0]=0
>pucRegBuffer[1]=0





- 示例2

> 000020-Tx:01 03 **00 01 00 09** D4 0C
> 000021-Rx:01 03 12 00 0C 00 0D 00 04 00 05 00 06 00 07 00 08 00 09 00 0A 09 68

串口打印信息：

> eMBRegHoldingCB
> usAddress =2
> usNRegs = 9
> pucRegBuffer[0]=1
> pucRegBuffer[1]=0

**所以在使用该代码时：需要将PLC的地址转换为MCU地址，PLC的起始地址为1；**

```c
USHORT usAddress							# PLC地址
USHORT usRegIndex = usAddress - 1; # 将PLC地址平移到从0开始 
```

