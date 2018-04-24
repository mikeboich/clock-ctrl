/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
 * GPS and time-keeping routinesL
*/
#include <RTC_1.h>
#include "time.h"
int sentence_avail;
char sentence[256];

// number of pps pulses required before we start using gps pss for timekeeping:
#define REQUIRED_PULSES 4

void init_gps();
void send_command(char *s); // send a command with checksum and crlf

time_t rmc_sentence_to_unix_time(char *sentence);

void invalidate_gps_pps();

float get_lat_or_long(int select);

// state machine that consumes characters and constructs sentence(s):
void consume_char(char c);
void set_rtc_to_gps();

void offset_time(RTC_1_TIME_DATE *t, int hours);
void increment_date(RTC_1_TIME_DATE *t, int incr);
void decrement_time(RTC_1_TIME_DATE *t, int hours);
void decrement_date(RTC_1_TIME_DATE *t);

/* [] END OF FILE */
