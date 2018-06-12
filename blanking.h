/*  blanking.c

 Copyright (C) 2016 Michael Boich

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 Routines to control blanking of CRT beam
 Major change from original scheme of octets.
 Now we use fine-grained timers to set and clear the blanking signal,
 which gives us finer control of where the beam is on and off
 *******************************************************************************/

#include <device.h>
#include <stdlib.h>

// Define bit constants for TimerReg control register:
// timer clock in MHz:
#define TIMER_CLK_FREQ 4
// phase lead in microseconds:
#define PHASE_LEAD_US 8
// phase lead where one period = 256:
#define PHASE_LEAD_DDS 64
#define FILTER_LAG 1

#define ON_TIMER_ENABLE 1
#define BEAM_ON 2
#define OFF_TIMER_ENABLE 4
#define BEAM_OFF 8
#define DDS_ENABLE 16
#define TIMERS_RESET 32
#define LOAD_CTRL 64
#define DDS_RESET 128

// define some macros to save call overhead:


void beam_on_now();
void beam_off_now();
void enable_timers();
void disable_timers();
void reset_timers();
void set_timers_from_mask(uint8 mask);
void x_set_timers_from_mask(uint8 mask);
void set_timers_for_line();
void enable_dds();
void disable_dds();
void reset_dds();
void dds_load();


/* [] END OF FILE */
