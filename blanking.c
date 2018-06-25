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
#include "font.h"
#include "draw.h"

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
void reset_dds(){
   uint8 reg = Timer_Reg_Read();
   reg = reg & DDS_RESET;
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
    for(index=0;index<8;index++, b = b >> 1){
        bits[index] = (b & 0x01) ? 1 : 0;             // bits are in hardware-draw-order now (clockwise from 3 o'clock)
    }
    
    for(index=0;index<8;index++){
        if(bits[index]){ 
            if( start_octant > index)  start_octant = index;
             if(stop_octant < index) stop_octant = index;
        }
    }
    int lead_time = PHASE_LEAD_US * TIMER_CLK_FREQ + FILTER_LAG*TIMER_CLK_FREQ;
    // for now, we're going to break on non-contiguous masks, by just drawing from the earliest on-time
    // to the latest off-time
    // each octant is 4 microseconds, which is timer_clk * 4 clocks
    //on_time = 4*TIMER_CLK_FREQ*start_octant + lead_time + FILTER_LAG;
    on_time = 4*TIMER_CLK_FREQ*start_octant+1*TIMER_CLK_FREQ + lead_time + 2;
    //off_time = 4*TIMER_CLK_FREQ*(stop_octant+1) - 1 + lead_time + FILTER_LAG;
    off_time = 4*TIMER_CLK_FREQ*(stop_octant+1)+2*TIMER_CLK_FREQ + lead_time-3;
    
    // intervention for the 0xff case so timers don't collide:
    if(mask == 0xff){
        on_time = 0;
        off_time = 8*4*TIMER_CLK_FREQ-1;
    }
    else{ 
        if(mask==0){
            on_time = off_time = 1;
        }
    }
    Z_On_Timer_WriteCounter(on_time);
    Z_On_Timer_WritePeriod(32*TIMER_CLK_FREQ-1);
    Z_Off_Timer_WriteCounter(off_time);
    Z_Off_Timer_WritePeriod(32*TIMER_CLK_FREQ-1);

}
void x_set_timers_from_mask(uint8 mask){
   uint8 hi_nybble = (mask >> 4) & 0x0f;
   uint8 low_nybble = mask & 0x0f;
   if(hi_nybble==low_nybble){
     set_timers_from_mask(0xC0);
    
    // back the timers up by one quadrant:
     Z_On_Timer_WriteCounter(Z_On_Timer_ReadCounter()+4*TIMER_CLK_FREQ);
     Z_Off_Timer_WriteCounter(Z_Off_Timer_ReadCounter()+4*TIMER_CLK_FREQ);
    
    // halve the periods:
     Z_On_Timer_WritePeriod(16*TIMER_CLK_FREQ-1);  // half the period of normal
     Z_Off_Timer_WritePeriod(16*TIMER_CLK_FREQ-1);
}
}

void set_timers_for_line(){
    Z_On_Timer_WriteCounter(1);
    Z_Off_Timer_WriteCounter(31*TIMER_CLK_FREQ);
}

void mask_to_bits(unsigned char  mask, unsigned int *result){
  unsigned int  bits[8];
  int i;

  for(i=0;i<8;i++){
    result[i] = mask & 0x80 ? 1 : 0;
    mask = mask << 1;
  }
}

void on_times(unsigned int  *bits, unsigned int *result){
  int i;

  for(i=0;i<8;i++){
    result[i] = ((bits[i]==1 && i==0) || (bits[i] == 1 && bits[i-1]==0)) ? 1 : 0;
  }
}

void off_times(unsigned int  *bits, unsigned int *result){
  int i;

  for(i=0;i<8;i++){
    result[i] = ((bits[i] == 1 && i == 7) || (bits[i]==1 && bits[i+1]==0)) ? 1 : 0;
  }
}

int  contig_masks(unsigned char mask, unsigned int *results){
  int result_index=0;
  unsigned int bits[8];
  unsigned int powers_of_two[8] = {1,2,4,8,16,32,64,128};
  int i;
  
  mask_to_bits(mask, bits);

  results[0] = 0;
  for(i=7;i>=0;i--){
    if(bits[i]){
      if(i!=7 && results[result_index] && bits[i+1] == 0) {
	result_index +=1;
	results[result_index] = 0;
      }
      results[result_index] |= powers_of_two[7-i];
      //printf("interim result = %i\n",results[result_index]);
    }
  }
  return result_index+1;
}

/* This function transforms a buffer of segments into a similar buffer, but with
   all octants contiguous.  i.e. one timer-on/timer-off pair for each segment.
   This requires splitting segments with non-contiguous blanking into multiple segments.
*/

void process_buffer(int src_buf){
    //seg_or_flag scratch_buffer[BUF_ENTRIES];
    
    // first copy the src buffer into the scratch buffer:
    clear_buffer(AUX_BUFFER);
    copyBuf(src_buf,AUX_BUFFER);
    
    // now AUX_BUFFER is the src, and the original src_buf is the destination:
    seg_or_flag *src = seg_buffer[AUX_BUFFER];
    seg_or_flag *dst = seg_buffer[src_buf];
    unsigned int masks[5];
    unsigned int i;
    unsigned int n_masks = 0;
    
    int num_entries = 0;
    while(src->flag !=255 && num_entries <= BUF_ENTRIES){
        contig_masks(src->seg_data.mask,&n_masks);
        for(i=0;i<n_masks;i++){
            src->seg_data.mask = masks[i];
            (dst++)->seg_data = (src++)->seg_data;
            num_entries += 1;
        }
        //src++;
    }
    dst->flag = 255;
}
/* [] END OF FILE */
