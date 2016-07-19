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
#include "prefs.h"

int init_prefs(){
    int flag;
    EEPROM_1_Start();
    CyDelay(1);
    flag = EEPROM_1_ReadByte(PREFS_FLAGS_OFFSET);
    if(flag == PREFS_INITIALIZED){
        return(1);
    }
    else{
        return(format_pref_memory());
        CyDelay(1);
    }
}
int format_pref_memory(){
 EEPROM_1_WriteByte(PREFS_FLAGS_OFFSET,PREFS_INITIALIZED);  // write magic byte
 CyDelay(1);
 EEPROM_1_WriteByte(0xf9,GMT_OFFSET_OFFSET);
CyDelay(1);
 return(1);
};

// load the gmt offset from prefs memory:
int get_gmt_offset(){
    int x = EEPROM_1_ReadByte(GMT_OFFSET_OFFSET);
    // doing my own twos complement math for now, as I couldn't the compiler to!:
    if(x>128) {
        x = (x ^ 0xff) + 1;
        x = -x;
    }
    return (x);  
};

void set_gmt_offset(int x){
    EEPROM_1_WriteByte(x,GMT_OFFSET_OFFSET);
}
/* [] END OF FILE */
