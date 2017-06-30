//
//  JulianDay.h
//  Lunar Foundation
//
//  Created by Michael Boich on 9/29/09.
//  Copyright 2009 Self. All rights reserved.
//

// adapted from my iPhone code to run on Cypress PSOC 5LP.
// Ported from Objective C, and unable to use timezone extensions
// to the c time.h library

// we use the application's GMT offset to translate from local time to UTC
// so sunrise, sunset, etc. are reported in the local time of the clock
// regardless of the location of the point for which the calcs are made

#include <time.h>
#include <stdio.h>
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
