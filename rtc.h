#ifndef _RTC_H
#define _RTC_H
#include <stdbool.h>
//#include "user.h"

#define SEC_IN_MIN (uint32_t)60
#define SEC_IN_HOUR (uint32_t)(SEC_IN_MIN * 60)
#define SEC_IN_DAY (uint32_t)(SEC_IN_HOUR * 24)
#define SEC_IN_MONTH (uint32_t)(SEC_IN_DAY * 31)
#define SEC_IN_YEAR (uint32_t)(SEC_IN_MONTH * 12)

#define	RTC_WRITE_ADDRESS	0xA2
#define	RTC_READ_ADDRESS	0xA3

#define SLAVE_ADDRESS    0xA2

struct _RTC
{
	uint8_t h; // godzina
	uint8_t m; // minuta
	uint8_t s; // sekunda
	uint8_t w; // dzien tygodznia
	uint8_t day; // dzien
	uint8_t month; // miesiac
	uint8_t year; // rok
};

typedef struct
{
	unsigned char hour;
	unsigned char minute;
	unsigned char second;

	unsigned int year;
	unsigned char month;
	unsigned char day;
	unsigned char weekday;

} _RTC_DATE_TIME;

void rtc_init(void);
void rtc_read_date_time(void);

//unsigned char rtc_HexToBcd(unsigned char p_bData);
//unsigned char rtc_BcdToHex(unsigned char p_bData);

//void rtc_send_data_PCF8563(unsigned char reg, unsigned char data,	unsigned char ilosc);
//unsigned char rtc_read_data_PCF8563(unsigned char reg);
unsigned char rtc_if_read_date_time(void);
void rtc_read_date_time_enable(void);
void rtc_read_date_time_disable(void);
void rtc_save_date_time();
//unsigned char rtc_getWeekDay(_RTC_DATE_TIME* p_sDate);
//void rtc_SET_NEW_CLOCK(void);
//unsigned char rtc_softrtcGetDaysInMonth( _RTC_DATE_TIME* p_sDate);
uint32_t packtime(struct _RTC* rtc);
void unpacktime(uint32_t time, struct _RTC* rtc);
void log_save(void);
bool log_read(void);
uint8_t rtc_softrtcGetDaysInMonth(uint8_t, uint8_t );
int rtcCompareTimeWithDeadZone(struct _RTC * time0, struct _RTC * time1, uint32_t deadZone);

extern _RTC_DATE_TIME g_date_time;

#endif
