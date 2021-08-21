/*
 * bufusart.c
 *
 * Created: 2013-01-24 18:13:44
 *  Author: tmf
 */ 

#include "bufusart.h"
#include "usart.h"
#include <avr/pgmspace.h>
#include <util/atomic.h>

CircBuffer recBuf, sendBuf;
CircBuffer recBufESP8266, sendBufESP8266; //odzielne bufory nadawcze dla funkcji USART_send_buf_F (FLASH), USART_send_buf_F_ESP8266 (RAM), UART F0
volatile uint8_t cmdrec=false;
volatile uint8_t cmdrecESP8266=false;
volatile bool TxFlag=false;
volatile bool TxFlagESP8266=false;

//ESP8266 START
ISR(USARTF0_RXC_vect)
{
	uint8_t ch=USARTF0_DATA;
	if(ch=='\r')
	{
		ch=0;
		++cmdrecESP8266;
	}
	cb_Add(&recBufESP8266, ch);
}

ISR(USARTF0_TXC_vect)
{
	if(!cb_IsEmpty(&sendBufESP8266))
	{
		USARTF0_DATA=cb_Read(&sendBufESP8266);
		TxFlagESP8266=true;
	} else TxFlagESP8266=false;
}

void USART_send_buf_FLASH(CircBuffer *buf, const char *txt)
{
	uint8_t ch;
	while ((ch=pgm_read_byte(txt++))!=0) cb_Add(&sendBufESP8266, ch);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(!TxFlagESP8266) USARTF0_TXC_vect();
	}
}

//przerobiona funkcja wysy³aj¹ca dan¹ z RAM'u a nie FLASH jak pierwotna funkcja USART_send_buf_F
void USART_send_buf_RAM(CircBuffer *buf, const char *txt)
{
	uint8_t ch;
	while ((ch=*txt++)!=0) cb_Add(&sendBufESP8266, ch);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(!TxFlagESP8266) USARTF0_TXC_vect();
	}
}
//ESP8266 END

//analogiczna funkcja do celów testowych na USB
void USART_send_buf_F_USB_RAM(CircBuffer *buf, const char *txt)
{
	uint8_t ch;
	while ((ch=*txt++)!=0) cb_Add(&sendBuf, ch);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(!TxFlag) USARTD0_TXC_vect();
	}
}

//USB START
void USART_send_buf_F_USB(CircBuffer *buf, const char *txt)
{
	uint8_t ch;
	while ((ch=pgm_read_byte(txt++))!=0) cb_Add(&sendBuf, ch);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(!TxFlag) USARTD0_TXC_vect();
	}
}

ISR(USARTD0_RXC_vect)
{
	uint8_t ch=USARTD0_DATA;
	if(ch=='\r')
	{
		ch=0;
		++cmdrec;
	}
	cb_Add(&recBuf, ch);
}

ISR(USARTD0_TXC_vect)
{
	if(!cb_IsEmpty(&sendBuf))
	{
		USARTD0_DATA=cb_Read(&sendBuf);
		TxFlag=true;
	} else TxFlag=false;
}
//USB END

