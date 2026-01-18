/*
 * AJOS Real-Time Clock Driver
 * Reads time from CMOS RTC chip
 */

#include "../include/rtc.h"
#include "../include/io.h"

/* CMOS RTC ports */
#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

/* CMOS RTC registers */
#define RTC_SECONDS  0x00
#define RTC_MINUTES  0x02
#define RTC_HOURS    0x04
#define RTC_DAY      0x07
#define RTC_MONTH    0x08
#define RTC_YEAR     0x09
#define RTC_STATUS_A 0x0A
#define RTC_STATUS_B 0x0B

/*
 * Read a CMOS register
 */
static uint8_t cmos_read(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

/*
 * Check if RTC update is in progress
 * Returns 1 if update in progress, 0 otherwise
 */
static int rtc_update_in_progress(void) {
    outb(CMOS_ADDRESS, RTC_STATUS_A);
    return (inb(CMOS_DATA) & 0x80);
}

/*
 * Convert BCD to binary
 */
static uint8_t bcd_to_binary(uint8_t bcd) {
    return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}

/*
 * Initialize RTC
 */
void rtc_init(void) {
    /* Nothing special needed for basic time reading */
}

/*
 * Get current time from RTC
 */
void rtc_get_time(rtc_time_t* time) {
    uint8_t status_b;

    /* Wait for any update to complete */
    while (rtc_update_in_progress());

    /* Read time registers */
    time->seconds = cmos_read(RTC_SECONDS);
    time->minutes = cmos_read(RTC_MINUTES);
    time->hours   = cmos_read(RTC_HOURS);
    time->day     = cmos_read(RTC_DAY);
    time->month   = cmos_read(RTC_MONTH);
    time->year    = cmos_read(RTC_YEAR);

    /* Check status register B to determine format */
    status_b = cmos_read(RTC_STATUS_B);

    /* If not in binary mode (BCD mode), convert */
    if (!(status_b & 0x04)) {
        time->seconds = bcd_to_binary(time->seconds);
        time->minutes = bcd_to_binary(time->minutes);
        time->hours   = bcd_to_binary(time->hours & 0x7F) | (time->hours & 0x80);
        time->day     = bcd_to_binary(time->day);
        time->month   = bcd_to_binary(time->month);
        time->year    = bcd_to_binary(time->year);
    }

    /* Convert 12-hour to 24-hour if needed */
    if (!(status_b & 0x02) && (time->hours & 0x80)) {
        time->hours = ((time->hours & 0x7F) + 12) % 24;
    }
}

/*
 * Get current hours
 */
uint8_t rtc_get_hours(void) {
    rtc_time_t time;
    rtc_get_time(&time);
    return time.hours;
}

/*
 * Get current minutes
 */
uint8_t rtc_get_minutes(void) {
    rtc_time_t time;
    rtc_get_time(&time);
    return time.minutes;
}

/*
 * Get current seconds
 */
uint8_t rtc_get_seconds(void) {
    rtc_time_t time;
    rtc_get_time(&time);
    return time.seconds;
}
