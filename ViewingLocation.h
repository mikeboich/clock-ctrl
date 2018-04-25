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
#include "math.h"

struct location {
  float latitude, longitude;
  time_t viewing_date;
  int gmt_offset;
};



void init_location(struct location *l);
// time_t calcSunOrMoonRiseForDate(time_t the_date, int riseOrSet, int sunOrMoon);

