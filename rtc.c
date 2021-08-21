#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <avr/interrupt.h>
//#include "main.h"
#include "rtc.h"

#include "twi_master_driver.h"
#include "twi_slave_driver.h"
//#include "user.h"


//odczyt i zapis po bajcie !!! - PCF8563 nie umie wiêcej transmitowaæ

/*! Baud register setting calculation. Formula described in datasheet. */
#define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5)

/*! CPU speed 2MHz, BAUDRATE 100kHz and Baudrate Register Settings */
#define CPU_SPEED       2000000
#define BAUDRATE	40000
#define TWI_BAUDSETTING TWI_BAUD(CPU_SPEED, BAUDRATE)

TWI_Master_t twiMaster;
TWI_Slave_t twiSlave;

unsigned char g_rtc_buffer[8];

_RTC_DATE_TIME g_date_time;

//uint8_t rtc_softrtcGetWeekDay(_RTC_DATE_TIME * p_sDate);
static unsigned char rtc_getWeekDay(_RTC_DATE_TIME* p_sDate);

// dodana przeze mnie funkcja do ustawiania czasu
// void RTC_SetTime(void)
// {
// 	g_date_time.second	= 30;
// 	g_date_time.minute	= 59;
// 	g_date_time.hour	= 23;
// 	g_date_time.day		= 31;
// 	g_date_time.month	= 12;
// 	g_date_time.year	= 2016;
// 	g_date_time.weekday	= 0;
// 	rtc_save_date_time(&g_date_time);
// }

//DO SPRAWDZENIA CZY DZIALA DOBRZE RTC
// void RTC_SetTime(void)
// {
// 	g_date_time.second	= 30;
// 	g_date_time.minute	= 59;
// 	g_date_time.hour	= 23;
// 	g_date_time.day		= 31;
// 	g_date_time.month	= 12;
// 	g_date_time.year	= 2016;
// 	g_date_time.weekday	= 0;
// 	rtc_save_date_time(&g_date_time);
// }

static unsigned char rtc_BcdToHex(unsigned char p_bData)
{
	unsigned char m_bRetVal;

	m_bRetVal = (p_bData / 16) * 10;
	m_bRetVal += (p_bData % 16);
	return (m_bRetVal);
}

static unsigned char rtc_HexToBcd(unsigned char p_bData)
{
	unsigned char m_bRetVal;

	m_bRetVal = (p_bData / 10) * 16;
	m_bRetVal += (p_bData % 10);
	return (m_bRetVal);
}//extrtc_HexToBcd

static void TWID_SlaveProcessData(void)
{
	uint8_t bufIndex = twiSlave.bytesReceived;
	twiSlave.sendData[bufIndex] = (~twiSlave.receivedData[bufIndex]);
}

void rtc_init(void)
{
	PORTC.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc | PORT_ISC_BOTHEDGES_gc;
	PORTC.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc | PORT_ISC_BOTHEDGES_gc;

	TWI_MasterInit(&twiMaster, &TWIC, TWI_MASTER_INTLVL_LO_gc, (uint8_t) TWI_BAUDSETTING);

	TWI_SlaveInitializeDriver(&twiSlave, &TWIC, TWID_SlaveProcessData);
	TWI_SlaveInitializeModule(&twiSlave, SLAVE_ADDRESS, TWI_SLAVE_INTLVL_LO_gc);
}

static unsigned char rtc_read_data_PCF8563(unsigned char reg)
{
	unsigned char data = reg;

	TWI_MasterWriteRead(&twiMaster, SLAVE_ADDRESS, &data, 1, 0);

	while (twiMaster.status != TWIM_STATUS_READY)
	{
	}

	TWI_MasterWriteRead(&twiMaster, SLAVE_ADDRESS, &data, 0, 1);

	while (twiMaster.status != TWIM_STATUS_READY)
	{
	}

	return twiMaster.readData[0];
}

void rtc_read_date_time(void)//odczyt zegara na 8MHz trwa 1ms
{
	unsigned char i;

	for (i = 0; i < 7; i++)
	{
		g_rtc_buffer[i] = rtc_read_data_PCF8563(i + 2);
	}

	g_date_time.second = rtc_BcdToHex(g_rtc_buffer[0]) % 60;
	g_date_time.minute = rtc_BcdToHex(g_rtc_buffer[1]) % 60;
	g_date_time.hour = rtc_BcdToHex(g_rtc_buffer[2] & 0x3F) % 24;
	g_date_time.day = rtc_BcdToHex(g_rtc_buffer[3] & 0x3F);
	g_date_time.month = rtc_BcdToHex(g_rtc_buffer[5] & 0x1F);
	g_date_time.year = rtc_BcdToHex(g_rtc_buffer[6]);
	g_date_time.weekday = rtc_getWeekDay(&g_date_time);
	asm("nop");
}

static void rtc_send_data_PCF8563(unsigned char reg, unsigned char data, unsigned char ilosc)
{

	unsigned char t_data[2];
	t_data[0] = reg;
	t_data[1] = data;

	TWI_MasterWriteRead(&twiMaster, SLAVE_ADDRESS, &t_data[0], ilosc ? 2 : 1, 0);

	while (twiMaster.status != TWIM_STATUS_READY)
	{
	}
}

void rtc_save_date_time()
{
	unsigned char i;

	g_rtc_buffer[0] = rtc_HexToBcd(g_date_time.second);
	g_rtc_buffer[1] = rtc_HexToBcd(g_date_time.minute);
	g_rtc_buffer[2] = rtc_HexToBcd(g_date_time.hour);
	g_rtc_buffer[3] = rtc_HexToBcd(g_date_time.day);
	g_rtc_buffer[4] = rtc_HexToBcd(g_date_time.weekday);//rtc_softrtcGetWeekDay(&g_date_time);
	g_rtc_buffer[5] = rtc_HexToBcd(g_date_time.month);
	g_rtc_buffer[6] = rtc_HexToBcd(g_date_time.year % 2000);
	for (i = 0; i < 7; i++)
	{
		rtc_send_data_PCF8563(i + 2, g_rtc_buffer[i], 1);
	}
}
static bool rtc_softrtcIsLeapYear(uint8_t year)
{
	return ((year % 4) == 0);
}

uint8_t rtc_softrtcGetDaysInMonth(uint8_t m, uint8_t y)
{
	if (m == 2)
	{
		return rtc_softrtcIsLeapYear(y) ? 29 : 28;
	}
	else if (m <= 7)
	{
		return (m & 1) ? 31 : 30;
	}
	else
	{
		return (m & 1) ? 30 : 31;
	}
}

static unsigned char rtc_getWeekDay(_RTC_DATE_TIME* p_sDate)
{
	long int p, q, r;
	unsigned char w;

	p = (14 - p_sDate->month) / 12;
	q = p_sDate->month + 12 * p - 2;
	r = p_sDate->year - p;
	w = (((p_sDate->day + (31 * q) / 12 + r + r / 4 - r / 100 + r / 400)) % 7);

	return (w); //0 - NIEDZIELA, 1 - PONIEDZIAúEK
}

/**************************************************************************************************************************
 procedura:
 unpacktime		: procedura dekoduj¹ca datê zapisan¹ na liczbie 4-bajtowej
 **************************************************************************************************************************/
uint32_t packtime(struct _RTC* rtc)
{
	return rtc->s
	+ rtc->m * SEC_IN_MIN
	+ rtc->h * SEC_IN_HOUR
	+ (rtc->day - 1) * SEC_IN_DAY
	+ (rtc->month - 1) * SEC_IN_MONTH
	+ rtc->year * SEC_IN_YEAR;
};
/**************************************************************************************************************************
procedura:
unpacktime		: procedura dekoduj¹ca datê zapisan¹ na liczbie 4-bajtowej
**************************************************************************************************************************/
uint32_t time;
void unpacktime(uint32_t _time, struct _RTC* rtc)
{
	time = _time;
	rtc->year = time / SEC_IN_YEAR;
	time -= (rtc->year * SEC_IN_YEAR);
	rtc->month = time / SEC_IN_MONTH;
	time -= (rtc->month * SEC_IN_MONTH);
	rtc->day = time / SEC_IN_DAY;
	time -= (rtc->day * SEC_IN_DAY);
	rtc->h = time / SEC_IN_HOUR;
	time -= (rtc->h * SEC_IN_HOUR);
	rtc->m = time / SEC_IN_MIN;
	time -= (rtc->m * SEC_IN_MIN);
	rtc->s = time;

	rtc->month++;
	rtc->day++;
};
//==========================================================================================
// Zapis zegara rejestracji resetów
void log_save(void)
{
	unsigned char i;

	g_rtc_buffer[0] = rtc_HexToBcd(g_date_time.minute);
	g_rtc_buffer[1] = rtc_HexToBcd(g_date_time.hour);
	g_rtc_buffer[2] = rtc_HexToBcd(g_date_time.day);

	if(g_date_time.month > 6)
	{
		g_rtc_buffer[3] = rtc_HexToBcd(g_date_time.month - 6);
		g_rtc_buffer[3] |= 0x80;
	}
	else
	{
		g_rtc_buffer[3] = rtc_HexToBcd(g_date_time.month);
	}

	for (i = 0; i < 4; i++)
	{
		rtc_send_data_PCF8563(i + 9, g_rtc_buffer[i], 1);
	}
};
/*

//==========================================================================================
// Odczyt zegara rejestracji resetów
bool log_read(void)
{
	unsigned char i;

	for (i = 0; i < 4; i++)
	{
		g_rtc_buffer[i] = rtc_read_data_PCF8563(i + 9);
	}

	log_time.minute = rtc_BcdToHex(g_rtc_buffer[0] & 0x7F) % 60;
	log_time.hour = rtc_BcdToHex(g_rtc_buffer[1]  & 0x3F) % 24;
	log_time.day = rtc_BcdToHex(g_rtc_buffer[2] & 0x3F);
	log_time.month = rtc_BcdToHex(g_rtc_buffer[3] & 0x07);

	if(g_rtc_buffer[3] & 0x80)
	{
		log_time.month += 6;
	}
	
	// Sprawdzenie poprawnoœci danych zegara
	if(log_time.day > 31 || log_time.day == 0
	|| log_time.month > 12 || log_time.month == 0
	)
	{
		return false;
	}

	return true;
};







 *	@brief
 *		Porownanie dwoch czasow uwzgledniajac nieczulosc
 *	@param time0
 *		Czas 1
 *	@param time1
 *		Czas 2
 *	@param deadZone
 *		Nieczulosc wyrazona w sekundach
 *	@return
 *		-1	: time0 < time1
 *		0	: time0 = time1
 *		1	: time0 > time1
 */


ISR( TWIC_TWIM_vect)
{
	TWI_MasterInterruptHandler(&twiMaster);
}

ISR( TWIC_TWIS_vect)
{
	TWI_SlaveInterruptHandler(&twiSlave);
}

int rtcCompareTimeWithDeadZone(struct _RTC * time0, struct _RTC * time1, uint32_t deadZone)
{
	uint32_t time0Value = packtime(time0);
	uint32_t time1Value = packtime(time1);
	if(time1Value > time0Value)
	{
		if(time1Value - time0Value >= deadZone)
		{
			return -1;
		}
		return 0;
	}
	else
	{
		if(time0Value - time1Value >= deadZone)
		{
			return 1;
		}
		return 0;
	}
}
