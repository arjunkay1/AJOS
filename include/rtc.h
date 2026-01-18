#ifndef RTC_H
#define RTC_H

#include <stdint.h>

/* Time structure */
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} rtc_time_t;

/* Initialize RTC */
void rtc_init(void);

/* Get current time */
void rtc_get_time(rtc_time_t* time);

/* Get hours (0-23) */
uint8_t rtc_get_hours(void);

/* Get minutes (0-59) */
uint8_t rtc_get_minutes(void);

/* Get seconds (0-59) */
uint8_t rtc_get_seconds(void);

#endif
