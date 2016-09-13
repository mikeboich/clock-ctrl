 /*  main.c

    Copyright (C) 2016 Michael Boich

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    A re-creation of the David Forbes  vector-graphics clock, implemented on Cypress PSOC
    and coded in C.

*******************************************************************************/
#include <device.h>

#define PREFS_FLAGS_OFFSET 0
#define GMT_OFFSET_OFFSET 1
#define SWITCH_INTERVAL_OFFSET 2
#define PREFS_INITIALIZED 0xaa

#define N_PREFS 3  

typedef struct prefsStruct {
    uint8 prefs_initialized;
    int8 utc_offset;
    uint8 switch_interval;
    uint8 sync_to_60Hz;     // 0 = no sync, non-zero = sync to edges of 60Hz clock
} prefs;

typedef union {prefs prefs_data;
       uint8 prefs_bytes[N_PREFS];
} pref_object;

extern pref_object global_prefs;

extern int8 gmt_offset;
int init_prefs();
int format_pref_memory();
void flush_prefs();

/* [] END OF FILE */
