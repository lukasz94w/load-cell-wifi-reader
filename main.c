//UWAGA MUSIAŁEM WYŁĄCZYC FUSEBIT JTAGEN BO BLOKOWAŁ PINY 4,5,6,7 PORTU B!

#include "main.h"
#include "bufusart.h"
#include "RingBuffer.h"
#include "usart.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_CMD_LEN 25      //Maksymalna długość polecenia w bajtach

const char cmd1[] PROGMEM = {"ZEGAR"};
const char cmd2[] PROGMEM = {"TAROW"};
const char cmd3[] PROGMEM = {"SERW1"};
const char cmd4[] PROGMEM = {"SERW2"};
const char * const cmds[] PROGMEM = {cmd1, cmd2, cmd3, cmd4};

//zmienne do obslugi ADS1220, oraz obliczania napiecia zasilania
float nap_1; //zmienna zadeklarowana globalnie (gdy byla zadeklarowana w int_main() byla lokalna i nie byla widoczna w przerwaniu (np TCC1_OVF_vect)
float nap_2;
float nap_3;
float nap_4;
float nap_5;
float g;
uint32_t _u_24;  //jak w przykladzie powyzej
float bat_lvl;
float napiecie;

//zmienne do ustawiania czasu
unsigned char sekunda;
unsigned char minuta;
unsigned char godzina;
unsigned char dzien;
unsigned char miesiac;
unsigned int rok;

//numer rekordu
uint16_t nr_rekordu;

//flaga do obslugi przerwania co sekunde, musi byc volatile bo jest uzywana w przerwaniu
volatile uint8_t flaga_sek=0;

//inicjalizacja przerwania od przepelnienia TCC0 (nalezy uzyc funkcji gdy korzystam z wektora przerwania ISR(TCC0_OVF_vect)
void init_TCC0()
{
	//konfiguracja przerwania od przepelnienia
	TCC0.INTCTRLA     =    TC_OVFINTLVL_LO_gc; //przepełnienie ma generować przerwanie LO
	//PMIC.CTRL         =    PMIC_LOLVLEN_bm; //odblokowanie przerwań o priorytecie LO - robimy to w init() wiec tutaj wykomentowane
	//sei();

	//konfiguracja timera
	TCC0.CTRLB        =    TC_WGMODE_NORMAL_gc; //tryb normalny
	//TCC0.CTRLFSET     =    TC0_DIR_bm; //liczenie w dół
	TCC0.CTRLA        =    TC_CLKSEL_DIV1024_gc; //ustawienie preskalera i uruchomienie timera
	TCC0.PER = 2000; //wartosc rejestru PER (do ktorej zlicza timer i generuje przerwanie gdy zliczy) PER=2000 co sekunde, PER=20000 co 10 sekund
}

//przerwanie od przepelnienia TCC0, cialo funkcji obslugi przerwania
ISR(TCC0_OVF_vect)
{
	flaga_sek=1;
}

void init ()
{
	//przetwornik ADC
	//UWAGA MUSIAŁEM WYŁĄCZYC FUSEBIT JTAGEN BO BLOKOWAŁ PINY 4,5,6,7 PORTU B!
	PORTB.DIRCLR = _ADCIN; //konfiguracja PIN5 jako wejscie (do pomiaru napiecia)
	//obsluga diody STATUS
	PORTD.DIRSET = STATUS_LED; //ustawienie kierunku wyjsciowego w celu mozliwosci sterowania dioda STATUS
	//I2C
	PORTC.DIRSET = _SDA | _SCL; //ustawienie jako wyjscie
	//obsluga klawiszy
	PORTE.PIN0CTRL = PORT_OPC_PULLUP_gc; //podciagniecie do zasilania dla KEY_2
	PORTE.PIN1CTRL = PORT_OPC_PULLUP_gc; //podciagniecie do zasilania dla KEY_1
	//inicjalizacja RTC
	rtc_init();

	//zalaczenie SPI
	SpiInit();
	PMIC.CTRL |= PMIC_LOLVLEN_bm; //zalaczenie i odblokowanie przerwan (miedzy innymi do RTC i do init_TCC0())
	sei();
	
	//Inicjalizacja WiFi
	//NMCU_EN (PD4) wyjscie i stan wysoki ////NMCU_EN (PD3) wyjscie i stan wysoki bylo
	PORTD.DIRSET = PIN4_bm; //ustawienie jako wyjscie
	PORTD.OUTSET = PIN4_bm; //ustawienie stanu wysokiego
	//NMCU_RST (PD5) wyjscie i stan wysoki pozniej niski reset //NMCU_RST (PD4) wyjscie i stan wysoki pozniej niski reset BYLO
	PORTD.DIRSET = PIN5_bm; //wyjscie
	PORTD.OUTSET = PIN5_bm; //stan wysoki
	PORTD.OUTCLR = PIN5_bm; //stan niski - RESET
	PORTD.OUTSET = PIN5_bm; //stan wysoki
	my_delay_ms(50); //odczekanie 50ms
	//NMCDU_D3 (PF0) wyjscie i stan wysoki
	PORTF.DIRSET = PIN0_bm; //ustawienie jako wyjscie
	PORTF.OUTSET = PIN0_bm; //ustawienie stanu wysokiego
	//NMCDU_D0 (PF1) wyjscie i stan wysoki
	PORTF.DIRSET = PIN1_bm; //ustawienie jako wyjscie
	PORTF.OUTSET = PIN1_bm; //ustawienie stanu wysokiego
		
	//inicjalizacja UART
	setUpSerial();
}

void ads_offset_calibration()
{
	Setup_ADS1220(ADS1220_MUX_SHORTED, ADS1220_OP_MODE_NORMAL, ADS1220_CONVERSION_SINGLE_SHOT,
	ADS1220_DATA_RATE_20SPS, ADS1220_GAIN_128, ADS1220_USE_PGA, ADS1220_IDAC1_DISABLED,
	ADS1220_IDAC2_DISABLED, ADS1220_IDAC_CURRENT_OFF, ADS1220_VREF_EXT_REF1_PINS);
}

void ads_init()
{
	Setup_ADS1220(ADS1220_MUX_AIN1_AIN2, ADS1220_OP_MODE_NORMAL, ADS1220_CONVERSION_SINGLE_SHOT,
	ADS1220_DATA_RATE_20SPS, ADS1220_GAIN_128, ADS1220_USE_PGA, ADS1220_IDAC1_DISABLED,
	ADS1220_IDAC2_DISABLED, ADS1220_IDAC_CURRENT_OFF, ADS1220_VREF_EXT_REF1_PINS);
	//unsigned char b[4]={0x3e,0x04,0x98,0x00}; //dane z noty str 57 reczne ustawienie z pominieciem funkcji setup
	//ADS1220_Write_Regs ((&b), 0, 4);
}

//w tej funkcji przekazujemy gain - jest on w sumie niepotrzebny do niczego bo juz nie dzielimy przez niego (wzmocnienie okreslamy TYLKO w funkcji Setup_ADS1220)
float ads_measure()
{
	//unsigned char b[1]={0xe};
	//ADS1220_Write_Regs ((&b), 0, 1);
	unsigned char a[3]; //tablica 3 bajtowa do zapisu 24 bitowego wyniku z przetwornika
	while (PORTE.IN & _DRDY)	{;}	//oczekiwanie na nowe dane (czy _DRDY zmienilo stan na niski) użycie przy continues mode chociaz bez debaga tez dziala na single shoot
	//my_delay_ms(50); //zamiast oczekiwania na zakończenie konwersji w trybie single shot
	ADS1220_Get_Conversion_Data(&a);// get the data from the ADS1220
	_u_24=(a[0]<<8|a[1]); //zmienna do przechowywania wyniku z ADS1220
	_u_24=(_u_24<<8)|a[2]; //załadowanie tablicy 3x8bit do jednego elementu 24bit
	_u_24=(_u_24 & 0x00FFFFFF); //wyzerowanie bitów 31-24
	
	//POPRAWIONO ZAKRESY W TEJ FUNKCJI TERAZ JEST WSZYSTKO DOBRZE)
	
	//TU BYLO ZLE JEDNAK!!!!
	//float nap_1= sd24conv(_u_24, 1, 0x00800000, 0x00ffffff, -5, -0.00000059604644775390625, 0x0, 0x007FFFFF, 0, 4.99999940395355224609375); //wywolanie funkcji sd24conv (trzeba pamietac ze dzieli nam tylko wynik w woltach, binarnie nie dzielimy!)
	float nap_1= sd24conv(_u_24, 1, 0x00800000, 0x00ffffff, -0.0390625, -0.00000000465661287, 0x0, 0x007FFFFF, 0, 0.0390624953433871);
	
	//tu nie powinno byc nap_1 dla czytelnosci kodu ale dziala
	return nap_1;
}

//konwersja danych binarnc z sd24 na napięcie
//zwroci wartość napięcia ze znakiem 0V do 5V typu float
//przyjmie uint32 aktualnie odczytana wartość, uint8 wzmocnienie,
//uint32 binarnie poczatek 1 zakresu, uint32 binarnie koniec 1 zakresu, float wartość napięcia dla poczatku 1 zakresu, float wartość napięcia dla konca 1 zakresu
//uint32 binarnie poczatek 2 zakresu, uint32 binarnie koniec 2 zakresu, float wartość napięcia dla poczatku 2 zakresu, float wartość napięcia dla konca 2 zakresu
float sd24conv (uint32_t actual_bin, uint8_t gain, //gain w sumie niepotrzebny tutaj jest ale niech bedzie (30 GRUDNIA)
uint32_t start_1range_bin,  uint32_t end_1range_bin,  float start_1range_voltage, float end_1range_voltage,
uint32_t start_2range_bin,  uint32_t end_2range_bin,  float start_2range_voltage, float end_2range_voltage)
{
	if ((actual_bin >= start_1range_bin) && (actual_bin <= end_1range_bin)) //sprawdzenie czy wynik miesci sie w pierwszym zakresie
	{
		float  result_X=((end_1range_voltage-start_1range_voltage)/(end_1range_bin-start_1range_bin))*(actual_bin-start_1range_bin)+start_1range_voltage; //obliczenia ze wzoru: [(y2-y1)/(x2-x1)]*(x-x1)+y1
		return result_X;
	}
	
	else
	{
		float  result_Y=((end_2range_voltage-start_2range_voltage)/(end_2range_bin-start_2range_bin))*(actual_bin-start_2range_bin)+start_2range_voltage;
		return result_Y;
	}
}

void battery_watchdog()
{
	// 	if(napiecie<=3.2)
	// 	{
	// 		power_down();
	// 	}
}

// void RTC_SetTime()
// {
// 	g_date_time.second	= sekunda;
// 	g_date_time.minute	= minuta;
// 	g_date_time.hour	= godzina;
// 	g_date_time.day		= dzien;
// 	g_date_time.month	= miesiac;
// 	g_date_time.year	= rok;
// 	g_date_time.weekday	= 1;
// 	rtc_save_date_time(&g_date_time);
// }

void RTC_SetTime() //testowo!
{
	g_date_time.second	= 30;
	g_date_time.minute	= 59;
	g_date_time.hour	= 23;
	g_date_time.day		= 31;
	g_date_time.month	= 12;
	g_date_time.year	= 2016;
	g_date_time.weekday	= 1;
	rtc_save_date_time(&g_date_time);
}

void pomiar()
{
	ads_offset_calibration ();
	ADS1220_Start();
	nap_1=ads_measure();
	ads_init();
	ADS1220_Start();
	nap_2=ads_measure();
	nap_5 = nap_2-nap_1; //wyliczamy napiecie
}


void funkcja_ramka_danych()
{
		rtc_read_date_time(); //odczyt zegara z RTC (odczyt zegara na 8MHz trwa 1ms)
		bat_lvl=ReadADC(); //odczytane napiecie (z zakresu 0-4095 - trzeba to przeliczyc)
		napiecie = (((1*bat_lvl)/4096)*5.5454);
		battery_watchdog();
		pomiar();
		PORTD.OUTTGL = STATUS_LED; //miganie diody za kazdym razem gdy wysyla dane
}

bool GetToken(char txt[MAX_CMD_LEN], uint8_t len)
{
	char ch;
	for(uint8_t i=0; i<10; i++)        //Przepisz token do zmiennej txt w formacie NULLZ
	{
		if(cb_IsEmpty(&recBuf)) return false; //Błąd
		ch=cb_Read(&recBuf);
		if(ch==' ') ch=0;
		txt[i]=ch;
		if(ch==0) break;
	}
	return true;
}


uint8_t TranslateCommand()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)   //Licznik odebranych komend
	{
		if(cmdrec) --cmdrec;
	}
	
	uint8_t indeks;
	char txt[MAX_CMD_LEN];
	if(GetToken(txt, MAX_CMD_LEN)==false) return -1;  //Wystąpił błąd
	
	for(indeks=0; indeks<sizeof(cmds)/sizeof(cmds[0]); indeks++)  //Porównaj token z listą
	
	//if(strcmp_P(txt, cmds[indeks])==0) break;
	if(strcmp_P(txt, (char*)pgm_read_word(&cmds[indeks]))==0) break; //POPRAWKA Z FORUM ELEKTRODY!!!, wczesniej bez tego mozna bylo maksymalnie chyba dwie komendy rozumiec, pozostale olewal
	
	return indeks;
}


int32_t GetHEXArg()
{
	char txt[MAX_CMD_LEN];
	if(GetToken(txt, MAX_CMD_LEN)==false) return -1;
	return strtol(txt, NULL, 16);   //Przekonwertuj argument hex na liczbę
}


void startWifi()
{
	PORTD.OUTSET = PIN5_bm; //stan wysoki
	PORTD.OUTCLR = PIN5_bm; //stan niski - RESET
	my_delay_ms(200);
	PORTD.OUTSET = PIN5_bm; //stan wysoki
	my_delay_ms(5000);
	PORTD.OUTTGL = STATUS_LED;
	USART_send_buf_FLASH(&sendBufESP8266, PSTR("wifi.setmode(wifi.STATION)\r\n"));
	my_delay_ms(500);
	PORTD.OUTTGL = STATUS_LED;
	USART_send_buf_FLASH(&sendBufESP8266, PSTR("wifi.sta.config(\"linksys\", \"\")\r\n"));
	my_delay_ms(500);
	PORTD.OUTTGL = STATUS_LED;
	USART_send_buf_FLASH(&sendBufESP8266, PSTR("wifi.sta.connect()\r\n"));
	my_delay_ms(500);
	PORTD.OUTTGL = STATUS_LED;
	USART_send_buf_FLASH(&sendBufESP8266, PSTR("socket=net.createConnection(net.UDP,0)\r\n"));
	my_delay_ms(500);
	PORTD.OUTTGL = STATUS_LED;
	USART_send_buf_FLASH(&sendBufESP8266, PSTR("socket:connect(4023, \"192.168.1.102\")\r\n"));
	my_delay_ms(500);
	
	PORTD.OUTTGL = STATUS_LED;
	my_delay_ms(200);
	PORTD.OUTTGL = STATUS_LED;
	my_delay_ms(200);
	PORTD.OUTTGL = STATUS_LED;
	my_delay_ms(200);
	PORTD.OUTTGL = STATUS_LED;
	my_delay_ms(200);
	PORTD.OUTTGL = STATUS_LED;
	my_delay_ms(200);
	PORTD.OUTTGL = STATUS_LED;
	my_delay_ms(200);
}


int main(void)
{
	init(); //inicjalizacja peryferiów
	startWifi(); //inicjalizacja połączenia WiFi
	RTC_SetTime(); //testowe ustawienie czasu
	init_TCC0(); //zalaczenie co sekunde przerwania od przepelnienia licznika (wyzwala pomiar co sekunde)
	
	while (1)
	{
		if((flaga_sek)==1) //co sekundę zapalam flagę w przerwaniu - wyzwalam pomiar
		{
			funkcja_ramka_danych();
			
			//po UDP
			char tmp[40];
			//sprintf_P(tmp, PSTR("socket:send(\"%hhu:%hhu:%hhu;%.2f;%.2f|\")\n"), g_date_time.hour, g_date_time.minute, g_date_time.second, napiecie, nap_5);
			sprintf_P(tmp, PSTR("socket:send(\"%.10f\")\n"), nap_5);
			USART_send_buf_RAM(&sendBufESP8266, tmp);
			
			//po USB
			char tmp_USB[40];
			sprintf_P(tmp_USB, PSTR("%.10f"), nap_5);
			USART_send_buf_F_USB_RAM(&sendBuf, tmp_USB);
			
			flaga_sek=0; //kasowanie flagi
		}	
		
		//JAK WYSYŁAM PO USB DANE TO SWITCH MUSI BYC WYKOMENTWANY!
// 		if(cmdrec)
// 		{
// 			switch(TranslateCommand())
// 			{
// 				case 0 : USART_send_buf_F_USB(&sendBuf, PSTR("Ustawiono zegar!\r\n")); break;
// 				case 1 : USART_send_buf_F_USB(&sendBuf, PSTR("Wykonano tarowanie!\r\n"));break;
// 				case 2 : USART_send_buf_F_USB(&sendBuf, PSTR("Tryb serwis 1!\r\n"));break;
// 				case 3 : USART_send_buf_F_USB(&sendBuf, PSTR("Tryb serwis 2!\r\n"));break;
// 				default: USART_send_buf_F_USB(&sendBuf, PSTR("Nieznane polecenie!\r\n"));
// 			}
// 		}		
	}
}
