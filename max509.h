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

#include <device.h>
#include "font.h"
// Some definitions for the MAX509:
#ifndef max509_h
#define max509_h
#define DAC_Reg_A 0x0000
#define DAC_Reg_B 0x0400
#define DAC_Reg_C 0x0800
#define DAC_Reg_D 0x0C00

#define DAC_Pre_Load 0x0100
#define DAC_Load_Now 0x0300

#define DAC_LDAC 0x0000
    
uint8 ss_x_offset, ss_y_offset;
    
void set_DACfor_seg(seg_or_flag *s,uint8 x, uint8 y);

void strobe_LDAC();

void bringup_test();

#endif
/* [] END OF FILE */
