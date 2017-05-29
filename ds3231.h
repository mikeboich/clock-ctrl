/* ========================================
 *
    Copyright (C) 2016 Michael Boich

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    Routines for the Maxim DS3231 real time clock chip
 *
 * ========================================
*/

#include <device.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

time_t get_DS3231_time();
void setDS3231(time_t time_now);

/* [] END OF FILE */
