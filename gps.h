/*  

 Copyright (C) 2016-2018 Michael Boich

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

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
