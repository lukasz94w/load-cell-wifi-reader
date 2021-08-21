#ifndef _SPI_H
#define _SPI_H

#include "main.h" //do makrodefinicji potrzebne
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

void SpiInit(void);
unsigned char SPI_Write_Byte (unsigned char);

//nie dawalem tu propotypu bo w obu przypadkach wywala caly program
//void SPI_Write (unsigned char *outData,  unsigned char *inData, unsigned char length)
//void SPI_Write (unsigned char*,  unsigned char*, unsigned char)
#endif /* SPI_H_ */
