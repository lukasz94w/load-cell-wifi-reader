#ifndef USART_H_
#define USART_H_

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h> //dodane zeby dzialala funkcja bool usart_set_baudrate

bool usart_set_baudrate	(USART_t *, uint32_t, uint32_t);
void setUpSerial();

#endif /* USART_H_ */




