#ifndef USART_H
#define USART_H

#include <HAL.h>
#include <RingBuffer.h>

//размер буфера
#define SIZE_TX_BUF 16
#define SIZE_RX_BUF 16

BOOL USART_PutChar(u08 byte); //положить символ в буфер
extern void USART_Init(void); //инициализация usart`a
extern void USART_SendByte(u08 byte);
extern void USART_SendStr(u08 *data); //послать строку по usart`у
extern void USART_SendNum(s16 num);//послать число по usart`у
extern u08 USART_GetChar(void); //прочитать приемный буфер usart`a 
extern void USART_UDRE_Handler(void);
extern void USART_RXC_Handler(void); 

#endif //USART_H