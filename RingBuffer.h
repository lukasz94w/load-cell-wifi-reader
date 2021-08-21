/*
 * RingBuffer.h
 *
 * Created: 2013-01-22 23:10:31
 *  Author: tmf
 */ 


#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <stdint.h>
#include <stdbool.h>

#define CB_MAXTRANS  40         //Maksymalna liczba elementów bufora

typedef uint8_t CB_Element;     //Typ elementów w buforze

typedef struct
{
	uint8_t Beg;                       //Pierwszy element bufora
	uint8_t Count;                     //Liczba elementów w buforze
	CB_Element elements[CB_MAXTRANS];  //Elementy bufora
} CircBuffer;

bool cb_Add(CircBuffer *cb, CB_Element elem);
CB_Element cb_Read(CircBuffer *cb);

static inline bool cb_IsFull(CircBuffer *cb)
{
	return cb->Count == CB_MAXTRANS;
}

static inline bool cb_IsEmpty(CircBuffer *cb)
{
	return cb->Count == 0;
}

#endif /* RINGBUFFER_H_ */