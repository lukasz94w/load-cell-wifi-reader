#include "spi.h"

//funkcja inicjalizujaca SPI
void SpiInit()
{
	PORTE.DIRSET = _ADS_CS | ADS_MOSI | ADS_SCK; //ustawienie jako wyjscia linii MOSI SCK i CS
	PORTE.DIRCLR = ADS_MISO |_DRDY; //ustawienie jako wejscie MISO, DRDY
		
	PORTE.PIN3CTRL = PORT_OPC_PULLUP_gc; //podciagniecie do zasilania
    PORTE.PIN4CTRL = PORT_OPC_PULLUP_gc; //podciagniecie do zasilania 
	PORTE.PIN5CTRL = PORT_OPC_PULLUP_gc; //podciagniecie do zasilania 
	PORTE.PIN6CTRL = PORT_OPC_PULLUP_gc; //podciagniecie do zasilania 
	PORTE.PIN7CTRL = PORT_OPC_PULLUP_gc; //podciagniecie do zasilania 

	SPIE.CTRL = SPI_ENABLE_bm| //wlaczenie SPI
				SPI_MASTER_bm| //tryb master
				SPI_MODE_1_gc| //tryb 1  w takim pracuje ADS
				SPI_PRESCALER_DIV64_gc; // preskaler (czestotliwosc przetwarzania przetwornika: fclk/16 i jeszcze /64 wychodzi okolo 2kHz), w trybie TRUBO MODE fclk/8 ale MNIEJSZE SZUMY	

	PORTE.OUTCLR = _ADS_CS; //zalaczenie za³¹czone na sta³e
}

//funkcja wysylajaca po jednym bajcie dane na magistrali SPI (jeden bajt wysyla do mastera czeka na zakonczenie transmisji i odbiera jeden bajt ze slave - BUFOR KOLOWY)
unsigned char SPI_Write_Byte (unsigned char data)
{
SPIE.DATA = data;
while(!(SPIE.STATUS & SPI_IF_bm)){;}
return  SPIE.DATA;
}
	
//funkcja do zapisu danych (zapisuje do slave i odczytuje dane ze slave - wykorzystywana jest w funkcji 
void SPI_Write (unsigned char *outData,  unsigned char *inData, unsigned char length)
{
	unsigned char i;
	for (i=0; i<length; i++)
	{
		inData[i] = SPI_Write_Byte (outData[i]); //zapisywanie i odczytywanie bajt po bajcie (do outData zapisuje kolejne bajty i odbiera bajty z inData (od slave))
	}
	_delay_us(50); //odczekanie 50us
}


