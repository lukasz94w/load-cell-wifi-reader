#include "adc.h"

uint8_t offset_adc=190;
uint16_t ADCvalues[10]; //tablica na wartosci odczytanych napiec z ADC
uint16_t min, max; //zmienne przechowujace wartosci minimalne i maksymalne (skrajne wartosci)
uint16_t sum; //zmienna pomocnicza do przechowywania sumy napiec z 10 probek
uint16_t o; //odczytana wartosc napiecia
uint16_t srednia; //zmienna przechowujaca srednie napiecie z 8 probek (po odrzuceniu dwoch skrajnych)

//mozna zmniejszyc zuzycie energii przez zmniejszenie czestotliwosci probkowania (liczby sampli na sekunde)
//ADCB.CTRLB | = ADC_CURRENTLIMITS_HIGH_gc; // 0x60

uint16_t ReadADC() //ADCMode - 1 dla single, 0 dla pomiaru roznicowego
//wybieramy tryb signed w tym trybie wejscie ujemne (-) ADC zostaje dolaczone do GND a do (+) dajemy ktores z napiec ADC0, ADC1 itd
{
	//zerowanie na poczatku sum
	sum=0;
	//petla dla 10 probek
	for(uint8_t i=0;i<10;i++)
	{
		//czytanie z przetwornika ADC
		
		if ((ADCB.CTRLA & ADC_ENABLE_bm) == 0)
		{
			ADCB.CTRLA = ADC_ENABLE_bm ; //zalaczenie ADC
			//ADCA.CTRLB = ADC_CONMODE_bm; //tryb ze znakiem. domyslnie jest wlaczony jest bez znaku
			//ADCA.REFCTRL = 0; // wybranie wenwetrznego napiecia  odniesienia 1V
			ADCB.REFCTRL =  ADC_REFSEL_INT1V_gc; //wybieramy napiecie 1V (30 GRUDNIA)
			//ADCA.REFCTRL =  ADC_REFSEL_INTVCC2_gc; //Vcc/2
			
			ADCB.EVCTRL = 0 ; //wylaczenie systemu zdarzen
			//ADCA.PRESCALER = ADC_PRESCALER_DIV32_gc ; //otrzymujemy 2MHz/32=62.5kHz (za malo powinno wynosic 100kHz-1.4MHz)
			ADCB.PRESCALER = ADC_PRESCALER_DIV8_gc ; //otrzymujemy 2MHz/8=250kHz
			//odczytujemy dane kalibracyjne z wewnetrznej pamieci EEPROM (rekompensuja niedopasowanie
			//poprawiaja liniowosc itp)
			ADCB.CALL = ReadSignatureByte(0x20) ; //wpisywanie do rejestu CALL (low)
			ADCB.CALH = ReadSignatureByte(0x21) ; //wczytanie do rejestru CALH (high)
			_delay_us(400); // czekaj co najmniej 25 taktów (25 clocks)
		}
		ADCB.CH0.CTRL = ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_SINGLEENDED_gc ; // tryb single (nie roznicowy), wzmocnienie wtedy (gain) zawsze rowne 1
		ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc; //ustawiamy kanal 5
		ADCB.CH0.INTCTRL = 0; //wylaczamy przerwania

		for(uint8_t Waste = 0; Waste<2; Waste++)
		{
			ADCB.CH0.CTRL |= ADC_CH_START_bm; //start konwersji
			while (ADCB.INTFLAGS==0); //czekaj na zakonczenie
			ADCB.INTFLAGS = ADCB.INTFLAGS;
		}
		//CH0RES WAZNE!!! mozna ustawic tez np CH1RES - rezultat z jakiegos kanalu 1
		o=(ADCB.CH0RES-offset_adc);   //odjêcie ofsetu ADC od wartoœci zmierzonej
		
		if (o>4095)	o=0;  //sprawdzenie czy po odjeciu offsetu nie przekrecilo licznika, maks dopuszczalna wartosc to 4095
		ADCvalues[i] = o; //wczytujemy wartosc z ADC

		if(i == 0) // jesli pierwszy pomiar
		{
			min=ADCvalues[i]; //daj go jako max i min na poczatek
			max=ADCvalues[i];
		}

		sum+=ADCvalues[i]; //sumuj napiecia z ADC
		if(ADCvalues[i]>max) max = ADCvalues[i]; //jesli > max to podmien
		if(ADCvalues[i]<min) min = ADCvalues[i]; // jesli < min podmien
	}
	srednia=(sum-(min+max))/8; //mamy sume 10 probek, odejmujemy od niej skrajne wartosci min i max, zostaje nam suma z 8 probek, liczymy z nich srednia
	return srednia;
	//sposob na obliczenie napiecia: 1 = 4095
	//1x=4.095, 4.095 zmienia sie
	//x=ADC/1 gdzie ADC wartosc odczytana (nasze o)
	//DZIELNIK 5.5:1!!!!!
}

uint8_t ReadSignatureByte(uint16_t Address)
{
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	uint8_t Result;
	__asm__ ("lpm %0, Z\n" : "=r" (Result) : "z" (Address));
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	return Result;
}
