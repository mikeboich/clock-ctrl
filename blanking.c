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
#include "blanking.h"

void beam_on_now(){
    uint8 reg = Timer_Reg_Read();
    reg |= TIMERS_RESET;            // reset timers...
    Timer_Reg_Write(reg);

    // reg = Timer_Reg_Read();
    reg &= ~(ON_TIMER_ENABLE | OFF_TIMER_ENABLE |BEAM_OFF);  //.. disable timers
    reg |= BEAM_ON;
    Timer_Reg_Write(reg);
    
}

void beam_off_now(){
  
    uint8 reg = Timer_Reg_Read();
    reg |= BEAM_OFF | TIMERS_RESET;          // turn off blanking
    
    // override timers and BEAM_ON bits by turning them off:
    reg = reg & ~(BEAM_ON | ON_TIMER_ENABLE | OFF_TIMER_ENABLE);
    Timer_Reg_Write(reg);
}

void enable_timers(){
    uint8 reg = Timer_Reg_Read();
    reg = reg | (ON_TIMER_ENABLE | OFF_TIMER_ENABLE);
    Timer_Reg_Write(reg);
}

void disable_timers(){
    uint8 reg = Timer_Reg_Read();
    reg = reg & ~(ON_TIMER_ENABLE | OFF_TIMER_ENABLE);
    Timer_Reg_Write(reg);
}

void reset_timers(){
    uint8 reg;
    reg = Timer_Reg_Read();
    reg = reg | (TIMERS_RESET );
}

void enable_dds(){
   uint8 reg = Timer_Reg_Read();
   reg = reg | (DDS_ENABLE);
   Timer_Reg_Write(reg);
}

void disable_dds(){
   uint8 reg = Timer_Reg_Read();
   reg = reg & ~DDS_ENABLE;
   Timer_Reg_Write(reg);
}

void dds_load(){
   uint8 reg = Timer_Reg_Read();
   reg = reg | LOAD_CTRL;
   Timer_Reg_Write(reg);

}

void set_timers_from_mask(uint8 mask){
    uint8 bits[8];
    int index;
    int start_octant=7, stop_octant=0;
    int on_time, off_time;
    
    // separate into 8 individual bits
    int b = mask;
    for(index=0;index<8;index++, b = b << 1){
        bits[index] = (b & 0x80) ? 1 : 0;             // bits are in hardware-draw-order now (clockwise from 3 o'clock)
    }
    
    for(index=0;index<8;index++){
        if(bits[index]){ 
            if( start_octant > index)  start_octant = index;
             if(stop_octant < index) stop_octant = index;
        }
    }
    int lead_time = PHASE_LEAD * TIMER_CLK_FREQ;
    // for now, we're going to break on non-contiguous masks, by just drawing from the earliest on-time
    // to the latest off-time
    // each octant is 4 microseconds, which is timer_clk * 4 clocks
    on_time = 4*TIMER_CLK_FREQ*start_octant + lead_time + FILTER_LAG;
    
    off_time = 4*TIMER_CLK_FREQ*(stop_octant+1) - 1 + lead_time + FILTER_LAG;
    
    Z_On_Timer_WriteCounter(on_time-1);
    Z_On_Timer_WritePeriod(32*TIMER_CLK_FREQ-1);
    Z_Off_Timer_WriteCounter(off_time-1);
    Z_Off_Timer_WritePeriod(32*TIMER_CLK_FREQ-1);

}

void set_timers_for_line(){
    Z_On_Timer_WriteCounter(64);
    Z_Off_Timer_WriteCounter(192);
}

/* [] END OF FILE */
