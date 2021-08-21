/*
 * RingBuffer.c
 *
 * Created: 2013-01-22 23:09:49
 *  Author: tmf
 */ 

#include "RingBuffer.h"

#include <util/atomic.h>

bool cb_Add(CircBuffer *cb, CB_Element elem)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(cb_IsFull(cb)) return false;         //Czy jest miejsce w kolejce?
		uint8_t end = (cb->Beg + cb->Count) % CB_MAXTRANS;
		cb->elements[end] = elem;              //Dodaj transakcj�
		++cb->Count;                           //Liczba element�w w buforze
	}
	return true;      //Wszystko ok
}

CB_Element cb_Read(CircBuffer *cb)
{
	CB_Element elem;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(cb_IsEmpty(cb)) return 0;       //Bufor pusty, nie mo�na zwr�ci� elementu
		elem = cb->elements[cb->Beg];
		cb->Beg = (cb->Beg + 1) % CB_MAXTRANS;
		-- cb->Count;                        //Zmniejszamy liczb� element�w pozosta�ych
	}		                                 //w buforze
	return elem;
}