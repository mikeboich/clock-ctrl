/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
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
