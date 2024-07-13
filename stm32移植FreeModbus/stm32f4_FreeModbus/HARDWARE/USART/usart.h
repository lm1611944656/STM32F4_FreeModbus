#ifndef __USART_H__
#define __USART_H__


//模式控制
#define RS485_TX_EN		PGout(8)	//485模式控制.0,接收;1,发送.

extern void usart1_init(uint32_t baud);
extern void usart1_send_bytes(uint8_t *pbuf,uint32_t len);
extern void rs485_init(uint32_t baud);
extern void rs485_send_bytes(uint8_t *pbuf,uint32_t len);


#endif



