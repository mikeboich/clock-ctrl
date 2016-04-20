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
#include <math.h>

// SPI constants:
#define TX_FIFO_NOT_FULL 4 

// Real time clock variables:
volatile int time_has_passed = 0;
int led_state = 0;  // we blink this once/second

// global screensaver offsets:
uint8 ss_x_offset=0, ss_y_offset=0;

int knob_position;
char test_string[] = "Hello!";

#define BUF_ENTRIES 150
#define DEBUG_BUFFER 3
seg_or_flag seg_buffer[4][BUF_ENTRIES];

// Routine to send data to the DAC over SPI.  Spins as necessary for full FIFO:
void setImmediate(uint16 spi_data){
  while((SPIM_1_ReadTxStatus() & TX_FIFO_NOT_FULL) == 0){   // spin if the fifo is full
    }
  SPIM_1_WriteTxData(spi_data);
}

// Drops the LDAC signal to load pre-buffered data in to the DAC:
void strobe_LDAC(){
    LDAC_Write(0u);
    CyDelayUs(1);
    LDAC_Write(1u);
}

typedef enum{textMode,analogMode,debugMode} clock_type;
clock_type display_mode=analogMode;

typedef enum{blank_unprimed,blank_primed,drawing}  draw_state;  // States of the draw loop/interrupt code

uint8 current_mask=0;
volatile draw_state current_state = blank_unprimed;
uint8 shape_to_mux[] = {2,0,1};
volatile int times_to_loop = 0;

volatile int cycle_count=0;  // poor man's timer
int last_refresh=0,loops_per_frame=0;

long total_time=0;

/* Define the start-of-segment interrupt routine 
   This routine loads the next 8 bit segment mask from the display list,
   and counts the number of drawing passes/segment.
*/

CY_ISR_PROTO(wave_started);

void wave_started(){
  isr_1_ClearPending();       // clear the interrupt
  cycle_count++;
  switch(current_state){

   case blank_unprimed:
      ShiftReg_1_WriteData(0x0);
    break;

    case blank_primed:
    current_state = drawing;
    break;
    
  case drawing:
    times_to_loop -= 1;
    if(times_to_loop==0){
	  ShiftReg_1_WriteData(0x0);  // blank on the next cycle
	  current_state = blank_unprimed;
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
    return seg_ptr->flag & 0x7f;                 // end flag - 0x80 + char width
}

// returns the width (sum of character widths) of a string:
uint8 stringWidth(char s[],uint8 scale){
    int width=0,index=0;
    
    while(*s){
        width += char_width(*s);
        s++;
    }
    return pin(width*scale); 
}

void compileString(char *s, uint8 y_coord,uint8 buffer_index,uint8 scale){  // turns a string into a display  list 
    seg_or_flag *src_ptr;
    seg_or_flag *dst_ptr;
    int num_segs=0;     // so we don't overrun our fixed-size buffer
    
    int kerning = (scale <= 2) ? 3: 2;
    int string_width = stringWidth(s,scale) + (strlen(s)-1)*kerning;;
    uint8 x = pin(128 - (string_width / 2));    //center on 128 wide for now
    dst_ptr = seg_buffer[buffer_index];
    while(*s && num_segs<BUF_ENTRIES){
        num_segs++;
        src_ptr = system_font[((uint8) *s)-32];
        while(src_ptr->seg_data.x_offset<0x80){
          dst_ptr->seg_data.x_offset = pin(scale*src_ptr->seg_data.x_offset+x);
          dst_ptr->seg_data.y_offset = pin(scale*src_ptr->seg_data.y_offset+y_coord);
          dst_ptr->seg_data.x_size = pin(scale*src_ptr->seg_data.x_size);
          dst_ptr->seg_data.y_size = pin(scale*src_ptr->seg_data.y_size);
          dst_ptr->seg_data.arc_type = src_ptr->seg_data.arc_type;
          dst_ptr->seg_data.mask = src_ptr->seg_data.mask;
          src_ptr++;
          dst_ptr++;
        }
        
        x = pin(x + scale*char_width(*s) + kerning);
        s++; 
    }
    dst_ptr->seg_data.x_offset = 0xff;       //used as a flag, but width not used
    dst_ptr->seg_data.mask=0;
}

// test case data:
seg_or_flag test_segs[] = {
    {128,128,255,255,cir,0xff},
    {128,128,255,255,pos,0x99},
    {128,128,96,96,neg,0x99},
    {128,128,96,0,pos,0x99},
    {128,128,0,96,pos,0x99},
    {255,255,0,0,cir,0xff},
};

void compileSegments(seg_or_flag *src_ptr, uint8 buffer_index){  // turns a string into a display  list 
    seg_or_flag *dst_ptr;
    int num_segs=0;     // so we don't overrun our fixed-size buffer
    
    dst_ptr = seg_buffer[buffer_index];
    while(src_ptr->seg_data.x_offset != 255 && num_segs<BUF_ENTRIES){
      num_segs++;
      dst_ptr->seg_data = src_ptr->seg_data;
      src_ptr++;
      dst_ptr++;
    }
    dst_ptr->seg_data.x_offset = 0xff;       // sentinel value
    dst_ptr->seg_data.mask=0;
}

void updateAnalogClock(int sec){
    seg_or_flag face[] = {{128,128,255,255,cir,0xff},
                            {128,128,255,255,pos,0x99},
                            {128,128,8,8,pos,0x99},
                            {125,210,00,30,pos,0x99},
                            {123,218,06,06,pos,0x99},
                            {.flag=0xff}};
    float angle = (sec/60.0)*2*M_PI;
    int quadrant =(sec / 15) + 1;
    
    switch (quadrant){
        
        case 1: {
            //angle += 3*M_PI_2;
            face[1].seg_data.x_size = (uint8) 128 * sin(angle);
            face[1].seg_data.y_size = (uint8) 128 * cos(angle);
            face[1].seg_data.x_offset+=face[1].seg_data.x_size/2;
            face[1].seg_data.y_offset += face[1].seg_data.y_size/2;;
            break;

        }
        
        case 2:{
            angle = angle - M_PI_2;
            face[1].seg_data.arc_type = neg;
            face[1].seg_data.x_size = (uint8) 128 * cos(angle);
            face[1].seg_data.y_size = (uint8) 128 * sin(angle);
            face[1].seg_data.x_offset+=face[1].seg_data.x_size/2;
            face[1].seg_data.y_offset -= face[1].seg_data.y_size/2;;
            break;
        }
       case 3: {
            angle -= M_PI;
            face[1].seg_data.x_size = (uint8) 128 * sin(angle);
            face[1].seg_data.y_size = (uint8) 128 * cos(angle);
            face[1].seg_data.x_offset -= face[1].seg_data.x_size/2;
            face[1].seg_data.y_offset -= face[1].seg_data.y_size/2;;
            break;

        }
            case 4:{
            angle -= 3*M_PI_2;
            face[1].seg_data.arc_type = neg;
            face[1].seg_data.x_size = (uint8) 128 * cos(angle);
            face[1].seg_data.y_size = (uint8) 128 * sin(angle);
            face[1].seg_data.x_offset -= face[1].seg_data.x_size/2;
            face[1].seg_data.y_offset += face[1].seg_data.y_size/2;;
            break;
        }
    }
    
    
    compileSegments(face,3);
}


void display_buffer(uint8 which_buffer){
    //long start_count = cycle_count;
    seg_or_flag *seg_ptr = seg_buffer[which_buffer];
    while(seg_ptr->seg_data.x_offset != 0xff){

        if(current_state==blank_unprimed){  
           uint8 int_status = CyEnterCriticalSection();
            
            preload_DAC_to_seg(seg_ptr,0,0);
            //CyDelayUs(12);
            AMux_1_Select(shape_to_mux[seg_ptr->seg_data.arc_type]);
            
            
            times_to_loop = (seg_ptr->seg_data.x_size>seg_ptr->seg_data.y_size) ? \
            seg_ptr->seg_data.x_size/6 : seg_ptr->seg_data.y_size/6;
            if(times_to_loop==0) times_to_loop = 1;
            if(seg_ptr->seg_data.arc_type == cir) times_to_loop *= 2;  // circles don't double up like lines

            
            //times_to_loop = 4;
            // performance measurement:
            if(which_buffer != DEBUG_BUFFER) loops_per_frame+=times_to_loop+1;
            
            
            
            //int length = sqrt(seg_ptr->seg_data.x_size + seg_ptr->seg_data.y_size);
            //times_to_loop = length/32 + 1;

            current_mask = seg_ptr->seg_data.mask;
            if(seg_ptr->seg_data.arc_type!=cir) current_mask=(current_mask ^ 0xff);
            ShiftReg_1_WriteData(current_mask);

            current_state = blank_primed;
            //CyDelayUs(4);
            strobe_LDAC();
            seg_ptr++;
          
           CyExitCriticalSection(int_status);


        }
    }
}

void initTime(){
    RTC_1_TIME_DATE *the_time = RTC_1_ReadTime();
    RTC_1_DisableInt();
    
    the_time->Month = 4;
    the_time->DayOfMonth = 20;
    the_time->DayOfWeek=4;
    the_time->Year = 2016;
    the_time->Hour = 9;
    the_time->Min = 23;
    the_time->Sec = 0;
    
    RTC_1_WriteTime(the_time);
    RTC_1_WriteIntervalMask(RTC_1_INTERVAL_SEC_MASK);
    RTC_1_EnableInt();
    
}

void updateTimeDisplay(){
    char time_string[32];
    char day_of_week_string[12];
    char date_string[15];
    char *day_names[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
    char *month_names[12] = {"Jan", "Feb", "Mar", "April","May","June","July","Aug","Sep","Oct","Nov","Dec"};

    RTC_1_TIME_DATE *the_time;
    the_time = RTC_1_ReadTime();
    int seconds = the_time->Sec;
    int minutes = the_time->Min;
    int hours = the_time->Hour;
    
    int day_of_week = the_time->DayOfWeek;
    int month = the_time->Month;
    int day_of_month = the_time->DayOfMonth;
    int year = the_time->Year;
    
    //update the interim screen-saver:
    ss_x_offset = (minutes) % 5;
    ss_y_offset =(minutes+2) % 5;
    
    
    sprintf(time_string,"%i:%02i:%02i",hours,minutes,seconds);
    compileString(time_string,0,0,3);

    
//    sprintf(time_string,"%i:%02i",hours,minutes);
//    compileString(time_string,0,0,4);

    sprintf(date_string,"%s %02i, %i",month_names[month-1],day_of_month,year);
    //sprintf(date_string,"April 15, 2016");
    compileString(date_string,114,1,1);
   // compileString("Hi Mom!",80,1,1);
    
     if(display_mode == analogMode || display_mode == debugMode) updateAnalogClock(seconds);
    
    char dw[12];
    sprintf(dw,"%s",day_names[day_of_week-1]);
    compileString(dw,176,2,2);

}

void diagPattern(){
    system_font[0] = (seg_or_flag*)&TestCircle;
    compileString(" ",0,0,1);
    for(;;) display_buffer(0);
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

 //start the real-time clock component (which is what this is all about..)
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

  //for(;;);
  //diagPattern();
compileSegments(test_segs,0);

 uint8 cc = 0;
// compileString("04/15/2016",0,0,1);
 compileString("4567",90,1,1);
 compileString("890",180,2,1);
 for(;;){
//    diag_test(QuadDec_1_GetCounter() % 104,32,32);
//     cc = (cc + 1);
//    if(cc>104) cc=0;

//    int phase = SixtyHz_Read();
//    while(SixtyHz_Read() == phase);   // wait for a 60Hz edge..
    
    if(display_mode == textMode){
        display_buffer(2);
        display_buffer(1);
        display_buffer(0);
    }
    else{
        display_buffer(3);
    }
    
    if(display_mode == debugMode){
        int elapsed = (cycle_count-last_refresh);
        char debug_str[32];
        sprintf(debug_str,"%i/%i/%i",31250/elapsed,elapsed,loops_per_frame);
        //sprintf(debug_str,"%i",31250/elapsed);
        compileString(debug_str,230,3,1);
        loops_per_frame=0;
        display_buffer(3);
        last_refresh = cycle_count;

    }
    if(time_has_passed){
        led_state = 1-led_state;
        LED_Reg_Write(led_state);
       updateTimeDisplay();
        time_has_passed = 0;     
    } 
    

}
   

#define SCOPE_DELAY_SHORT 96
}

/* [] END OF FILE */
