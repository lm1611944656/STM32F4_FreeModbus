#ifndef __USART_H__
#define __USART_H__


//ģʽ����
#define RS485_TX_EN		PGout(8)	//485ģʽ����.0,����;1,����.

extern void usart1_init(uint32_t baud);
extern void usart1_send_bytes(uint8_t *pbuf,uint32_t len);
extern void rs485_init(uint32_t baud);
extern void rs485_send_bytes(uint8_t *pbuf,uint32_t len);


#endif



