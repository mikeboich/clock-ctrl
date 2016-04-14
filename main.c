/*******************************************************************************
 * File: main.c
 *
 *
 * Description: 
 *  This is a source code for basic functionality of VDAC8.
 *  Main function performs following functions:
 *  1. Initializes the LCD and clears the display
 *  2. Start the VDAC8 component
 *  system_font3. Set the value through API
 *  4. Print test name and VDAC Range on the LCD
 *
 ********************************************************************************
 * Copyright 2012, Cypress Semiconductor Corporation. All rights reserved.
 * This software is owned by Cypress Semiconductor Corporation and is protected
 * by and subject to worldwide patent and copyright laws and treaties.
 * Therefore, you may use this software only as provided in the license agreement
 * accompanying the software package from which you obtained this software.
 * CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *******************************************************************************/

#include <device.h>
#include "font.h"
#include "max509.h"
#include <stdio.h>

// SPI constants:
#define TX_FIFO_NOT_FULL 4 

// Real time clock variables:
RTC_1_TIME_DATE time_record;
volatile int time_has_passed = 0;
int led_state = 0;  // we blink this once/second

// global screensaver offsets:
uint8 ss_x_offset=0, ss_y_offset=0;

int knob_position;
char test_string[] = "Hello!";
seg_or_flag seg_buffer[3][200];

// Rountine to send data to the DAC over SPI.  Spins as necessary for full FIFO:
void setImmediate(uint16 spi_data){
  while((SPIM_1_ReadTxStatus() & TX_FIFO_NOT_FULL) == 0){   // spin if the fifo is full
    }
  SPIM_1_WriteTxData(spi_data);
}

// Drops the LDAC signal to load pre-buffered data in to the DAC:
void strobe_LDAC(){
    LDAC_Write(0u);
    LDAC_Write(1u);
}



typedef enum{idle, looping, blanked}  draw_state;  // States of the draw loop/interrupt code

uint8 current_mask=0,next_mask=0;

volatile draw_state current_state = blanked;
uint8 cursor_x=0,cursor_y=0;
uint8 shape_to_mux[] = {2,0,1};
volatile int times_to_loop = 0;
volatile int primed=0;
shape next_shape=cir;
volatile long cycleCount=0;  // poor man's timer
long total_time=0;

/* Define the start-of-segment interrupt routine 
   This routine loads the next 8 bit segment mask from the display list,
   and will eventually program the DACS for xCenter, yCenter, xRadius, and yRadius as well.
*/

CY_ISR_PROTO(wave_started);

void wave_started(){
  isr_1_ClearPending();       // clear the interrupt
  cycleCount++;
  switch(current_state){

  case idle:
    current_mask = next_mask;
    ShiftReg_1_WriteData(current_mask);
    strobe_LDAC();
    AMux_1_Select(next_shape);
    current_state = looping;
    break;
  

   case blanked:
   if(primed){
    current_mask=next_mask;
    ShiftReg_1_WriteData(next_mask);
    AMux_1_Select(next_shape);

    strobe_LDAC();
    primed=0;
}

    break;
    
  case looping:
    times_to_loop -= 1;
    if(times_to_loop==0){
	  ShiftReg_1_WriteData(0x0);  // blank on the next cycle
	  current_state = blanked;
      }
    else {
	  ShiftReg_1_WriteData(current_mask);
      }
    break;
  }
}
    
  uint8 pin(int x){
    if(x<0)x=0;
    if(x>255)x=255;
    return x;
}


  void preload_DAC_to_seg(seg_or_flag *s,uint8 x, uint8 y){
    setImmediate(DAC_Reg_A | DAC_Pre_Load | s->seg_data.x_size);
    setImmediate(DAC_Reg_B | DAC_Pre_Load |s->seg_data.y_size);
    setImmediate(DAC_Reg_C | DAC_Pre_Load | (255-(s->seg_data.x_offset + x + ss_x_offset)));
    setImmediate(DAC_Reg_D | DAC_Pre_Load | (255-(s->seg_data.y_offset + y + ss_y_offset)));

  }



// returns the width of a single vector character:
int char_width(char c){
    seg_or_flag *seg_ptr = system_font[((uint8) c)-32];    // map from char code to segment list
    while(seg_ptr->seg_data.x_offset<0x80) seg_ptr++;       // skip over segments
    return seg_ptr->seg_data.x_offset-0x80;                 // end flag - 0x80 + char width
}

// returns the width (sum of character widths) of a string:
int stringWidth(char s[],uint8 mag){
    int width=0,index=0;
    
    while(s[index++]) width+=char_width(*s)+kerning;    
    return mag*width;
}

void compileString(char *s, uint8 y_coord,uint8 buffer_index,uint8 mag){  // turns a string into a display  list 
    seg_or_flag *src_ptr;
    seg_or_flag *dst_ptr;
    int num_segs=0;     // so we don't overrun our fixed-size buffer
    
    int string_width = stringWidth(s,mag);
    uint8 x = pin(128 - (string_width / 2));    //center on 128 wide for now
    //x=0;
    dst_ptr = seg_buffer[buffer_index];
    while(*s && num_segs<200){
        num_segs++;
        src_ptr = system_font[((uint8)*s)-32];
        while(src_ptr->seg_data.x_offset<0x80){
          dst_ptr->seg_data.x_offset = pin(mag*src_ptr->seg_data.x_offset+x);
          dst_ptr->seg_data.y_offset = pin(mag*src_ptr->seg_data.y_offset+y_coord);
          dst_ptr->seg_data.x_size = pin(mag*src_ptr->seg_data.x_size);
          dst_ptr->seg_data.y_size = pin(mag*src_ptr->seg_data.y_size);
          dst_ptr->seg_data.arc_type = src_ptr->seg_data.arc_type;
          dst_ptr->seg_data.mask = src_ptr->seg_data.mask;
          src_ptr++;
          dst_ptr++;
        }
        
        x = pin(x+ mag*(char_width(*s)+kerning));
        s++; 
    }
    dst_ptr->seg_data.x_offset = 0xff;       //used as a flag, but width not used
    dst_ptr->seg_data.mask=0;
}

void display_buffer(uint8 which_buffer){
    //long start_count = cycleCount;
    seg_or_flag *seg_ptr = seg_buffer[which_buffer];
    while(seg_ptr->seg_data.x_offset<0xff){

        if(!primed){  // otherwise wait until current_state==blanked
           uint8 int_status = CyEnterCriticalSection();
            
            preload_DAC_to_seg(seg_ptr,0,0);
            //CyDelayUs(12);
            
            next_shape = seg_ptr->seg_data.arc_type;
           next_mask = seg_ptr->seg_data.mask;
            if(seg_ptr->seg_data.arc_type!=cir) current_mask=(current_mask ^ 0xff);
            times_to_loop = 2;
            primed=1;
            seg_ptr++;
            //if(seg_ptr->seg_data.x_offset > 0xfe) seg_ptr = seg_buffer[which_buffer];
            //current_state = idle;
            
            
           CyExitCriticalSection(int_status);


        }
    }
}

void initTime(){
//    time_record.DayOfMonth = 14;
//    time_record.DayOfWeek = 5;
//    time_record.Hour = 9;
//    time_record.Min = 5;
//    time_record.Sec = 30;
//    time_record.Year = 2016;
    RTC_1_WriteHour(12);
    RTC_1_WriteMinute(52);
    RTC_1_WriteIntervalMask(RTC_1_INTERVAL_SEC_MASK);
    
}

int main() 
{
  /* Start up the SPI interface: */
  SPIM_1_Start();
    
  /* Start VDACs */
  VDAC8_1_Start();
  VDAC8_2_Start();
  VDAC8_3_Start();
    
  // Start opamp (drives analog reference):
  //Opamp_1_Start();
    
    
  /* Initialize the analog mux */
  AMux_1_Start();
  AMux_1_Select(0);
    
 
  /* Initialize the shift register: */
  ShiftReg_1_Start();
    
  //ShiftReg_1_WriteData(0xaa);  // not really needed..

// Start the quad decoder:
   QuadDec_1_Start();

    
  /* Initialize Interrupt: */
  isr_1_StartEx(wave_started);
  CyGlobalIntEnable;

// start the real-time clock component (which is what this is all about..)
   RTC_1_Start();
   LED_Reg_Write(1);
   initTime();
   

 /* initialize sysfont: */
  init_font();
    
  CyDelay(100);
    
  SPIM_1_WriteTxData(0x000);
  CyDelay(100);
  LDAC_Write(0u);
  CyDelayUs(5);
  LDAC_Write(1u);  // drop LDAC low to update DACS
  SPIM_1_WriteTxData(0x7ff );
  CyDelay(1);
  SPIM_1_WriteTxData(0x3ff);
  AMux_1_Select(1);
        
  SPIM_1_WriteTxData(DAC_Reg_C | DAC_Load_Now | 0x00);
  SPIM_1_WriteTxData(DAC_Reg_D | DAC_Load_Now | 0x00);

 uint8 cc = 0;
 compileString("12:35 PM",0,0,2);
 compileString("4/14/2016",90,1,2);
 compileString("Thursday",180,2,2);
 for(;;){
//    diag_test(QuadDec_1_GetCounter() % 104,32,32);
//     cc = (cc + 1);
//    if(cc>104) cc=0;
    while(SixtyHz_Read() != 0);  // sync to 60Hz for eliminate shimmer...
    //while(SixtyHz_Read() == 0);
    long start_time = cycleCount;
    display_buffer(0);
    display_buffer(1);
    display_buffer(2);
    if(time_has_passed){
        led_state = 1-led_state;
        LED_Reg_Write(led_state);
        time_has_passed = 0;
        
        int seconds = RTC_1_ReadSecond();
        int minutes = RTC_1_ReadMinute();
        int hours = RTC_1_ReadHour();
        
        char time_string[32];
        if(hours < 12)
          sprintf(time_string,"%i:%02i:%02iAM",hours,minutes,seconds);
        else
          sprintf(time_string,"%i:%02i:%02iPM",hours,minutes,seconds);
        
        compileString(time_string,0,0,2);
        
        // update screen saver offsets:
        //ss_x_offset = (ss_x_offset + 2) % 6;
        ss_y_offset = (ss_y_offset + 2) % 12;
    }
    long end_time = cycleCount;
    total_time = end_time-start_time;
    total_time +=1;
    
}
   

#define SCOPE_DELAY_SHORT 96
}

/* [] END OF FILE */
