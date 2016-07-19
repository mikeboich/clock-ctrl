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

#define PREFS_FLAGS_OFFSET 0
#define GMT_OFFSET_OFFSET 1
#define PREFS_INITIALIZED 0xaa

int init_prefs();
int format_pref_memory();
int get_gmt_offset();
void set_gmt_offset(int x);

/* [] END OF FILE */
