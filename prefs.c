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

/* prefs variables are stored in eeprom when power is off: */
pref_object global_prefs;

void load_prefs(){
    int offset = 0;
    while(offset <N_PREFS){
        global_prefs.prefs_bytes[offset] = EEPROM_1_ReadByte(offset);
        CyDelay(1);
        offset +=1;
    }
}

int init_prefs(){
    int flag;
    EEPROM_1_Start();
    CyDelay(1);
    flag = EEPROM_1_ReadByte(PREFS_FLAGS_OFFSET);
    if(flag == PREFS_INITIALIZED){
        load_prefs();       // copy prefs into the prefs struct
        return(1);
    }
    else{
        global_prefs.prefs_bytes[0] = 0xaa;
        global_prefs.prefs_data.utc_offset = -4;
        global_prefs.prefs_data.switch_interval = 10;
        flush_prefs();
        return(2);
        CyDelay(1);
    }
}


void flush_prefs(){
    int offset=0;
    EEPROM_1_UpdateTemperature();
    while(offset < N_PREFS){
      EEPROM_1_WriteByte(global_prefs.prefs_bytes[offset],offset);
      offset += 1;
    }
}
/* [] END OF FILE */
