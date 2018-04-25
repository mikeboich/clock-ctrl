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

// adapted from my iPhone code to run on Cypress PSOC 5LP.
// Ported from Objective C, and unable to use timezone extensions
// to the c time.h library

// we use the application's GMT offset to translate from local time to UTC
// so sunrise, sunset, etc. are reported in the local time of the clock
// regardless of the location of the point for which the calcs are made

#include <time.h>
#include <stdio.h>
#include <device.h>


char msg_buf[255];

#define PSOC

#ifdef PSOC
#define debugMsg(fmt_str, ...) sprintf(msg_buf,fmt_str, __VA_ARGS__); SW_Tx_UART_1_PutString(msg_buf); SW_Tx_UART_1_PutString("\n\r");
#else
#define debugMsg(fmt_str, ...)sprintf(msg_buf,fmt_str,##__VA_ARGS__); printf(msg_buf); 
#endif

time_t dateFromJulianDay(double jd);

time_t  midnightInTimeZone(time_t the_date, int gmt_offset);

double julianDayAt0000UT(time_t jd);
double julianDay(time_t the_date);
time_t calendarDateAt0000UT(time_t the_date, int gmt_offset);
double dynamicalTimeFromDate(time_t the_date);
time_t dateFromDynamicalTime(double dt);
double deltaTforDate(time_t the_date);
double bigThetaZeroInDegrees(time_t the_date);		// Sidereal time at 0H Universal Time at the Greenwich Meridian
double littleThetaZeroInDegrees(time_t the_date);		// Sidereal time at a given instant at the Greenwich Meridian
int secondsSinceMidnight(time_t the_date);

double reduce360(double x);
double degToRad(double deg);
double radToDeg(double rad);
double degToHours(double deg);
