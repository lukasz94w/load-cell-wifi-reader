#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
//aby odczytywac dane kalibracyjne nalezalo zainkludowac dwa pliki naglowkowe:
#include <avr/pgmspace.h>
#include <stddef.h>

uint16_t ReadADC();
uint8_t ReadSignatureByte(uint16_t);

#endif /* ADC_H_ */





