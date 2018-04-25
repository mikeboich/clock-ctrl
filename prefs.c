 /*  prefs.c

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
#include <stdio.h>
#include "prefs.h"

/* prefs variables are stored in eeprom when power is off: */
pref_object global_prefs;

void load_prefs(){
    uint offset = 0;
    while(offset <sizeof(pref_object)){
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
//  uncomment the 2 lines below for one-time initialization of ESN:
//        sprintf(global_prefs.prefs_data.esn,"P000");
//        flush_prefs();
        return(1);
    }
    else{
        global_prefs.prefs_bytes[0] = 0xaa;
        global_prefs.prefs_data.utc_offset = -4;
        global_prefs.prefs_data.switch_interval = 10;
        global_prefs.prefs_data.sync_to_60Hz = 0;
        global_prefs.prefs_data.use_gps = 1;
        global_prefs.prefs_data.minutes_till_sleep = 60;
        flush_prefs();
        return(2);
        CyDelay(1);
    }
}


void flush_prefs(){
    uint offset=0;
    EEPROM_1_UpdateTemperature();
    while(offset < sizeof(pref_object)){
      EEPROM_1_WriteByte(global_prefs.prefs_bytes[offset],offset);
      offset += 1;
    }
}
/* [] END OF FILE */
