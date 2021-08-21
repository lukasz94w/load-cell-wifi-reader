/*
 * bufusart.h
 *
 * Created: 2013-01-24 18:12:26
 *  Author: tmf
 */ 


#ifndef BUFUSART_H_
#define BUFUSART_H_

#include "RingBuffer.h"

extern CircBuffer recBuf;
extern CircBuffer sendBuf;
extern CircBuffer recBufESP8266;
extern CircBuffer sendBufESP8266;

extern volatile uint8_t cmdrec;
extern volatile uint8_t cmdrecESP8266;
extern volatile bool TxFlag;
extern volatile bool TxFlagESP8266;

void USART_send_buf_FLASH(CircBuffer *buf, const char *txt);
void USART_send_buf_RAM(CircBuffer *buf, const char *txt);
void USART_send_buf_USB_RAM(CircBuffer *buf, const char *txt);
void USART_send_buf_F_USB(CircBuffer *buf, const char *txt);

#endif /* BUFUSART_H_ */