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


#include "time.h"
#include "ViewingLocation.h"
time_t calcSunOrMoonRiseForDate(time_t the_date, int oneForRiseTwoForSet, int oneForSunTwoForMoon,struct location theLocation);
void calcLunarAzimuth(double *azimuthResult, double *elevationResult, double *fullness, double *rightAscensionResult, double *declinationResult, time_t theDate,struct location theLocation);
void calcSolarAzimuth(double *azimuthResult, double *elevationResult, double *rightAscensionResult, double *declinationResult, time_t theDate, struct location theLocation);
