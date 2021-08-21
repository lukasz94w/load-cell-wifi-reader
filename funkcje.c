#include "funkcje.h"

//funkcja omij¹j¹ca b³¹d braku mo¿liwoœci przekazania poprzez zmienna wartoœæi opoznienia do delay
void my_delay_ms(uint16_t n)
{
	while(n--)	{_delay_ms(1);}
}

//funkcja blink: liczba migniêæ, czas za³¹czenia ms, czas wy³¹czenia ms
void blink (uint8_t count, uint16_t time_on, uint16_t time_off)
{
	for (int i=0; i<count; i++)
	{
		PORTD.OUTSET = STATUS_LED; //zalaczenie diody status led
		my_delay_ms(time_on);
		PORTD.OUTCLR = STATUS_LED; //wylaczenie diody status led
		my_delay_ms(time_off);
	}
}

//funkcja power_down
void power_down()
{
// 	blink(7,60,80);
// 	PORTD.OUTCLR = PWR_ON; //wylaczenie zasilania
// 	blink(7,60,80);
// 	
// 	asm("cli");
// 	asm("jmp 0");
}

//funkcja do wylaczenia urzadzenia za pomoca KEY_1/KEY_2
void DEVICE_OFF()
{
	//sprawdza czy wcisniety przysk 2 do wylaczenia urzadzenia
	if(!(PORTE.IN & _KEY_2))
	{
		power_down();
	}	
}


