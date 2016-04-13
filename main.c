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

// SPI constants:
#define TX_FIFO_NOT_FULL 4 

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



typedef enum{start, looping, blanked}  draw_state;  // States of the draw loop/interrupt code

uint8 current_mask;
volatile draw_state current_state = blanked;
uint8 cursor_x=0,cursor_y=0;
uint8 shape_to_mux[] = {2,0,1};
volatile int times_to_loop = 0;
volatile int ready=1;

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

  case start:
    ShiftReg_1_WriteData(current_mask);
    strobe_LDAC();

    current_state = looping;
    break;
  

   case blanked:
    ready = 1;
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
    
  


  void preload_DAC_to_seg(seg_or_flag *s,int magnify,uint8 x, uint8 y){
    setImmediate(DAC_Reg_A | DAC_Pre_Load | magnify*s->seg_data.x_size);
    setImmediate(DAC_Reg_B | DAC_Pre_Load | magnify*s->seg_data.y_size);
    setImmediate(DAC_Reg_C | DAC_Pre_Load | (255-magnify*(s->seg_data.x_offset + x)));
    setImmediate(DAC_Reg_D | DAC_Pre_Load | (255-magnify*(s->seg_data.y_offset + y)));

  }

seg_or_flag *successor(seg_or_flag *s, seg_or_flag *startS){
    ++s;
    if (s->seg_data.x_offset < 0x80){
        return s;
    }
    else{
        return startS;
    }
}

void diag_test(uint8 char_code, uint8 cursor_x, uint8 cursor_y){
    long start_count = cycleCount;
   
   vector_font test_pat= {
    {64,64,128,128,cir,0xff},
    {00,00,32,32,cir,0xff},
    {00,127,32,32,cir,0xff},
    {127,0,32,32,cir,0xff},
    {127,127,32,32,cir,0xff},
    {.flag=0x88}

};
vector_font test_pat2={
{00,10,00,30,pos,0x99},
{12,10,00,30,pos,0x99},
{06,10,18,00,pos,0x99},

{22,06,10,12,cir,0xbf},
{22,06,15,00,pos,0x99},

{31,10,00,30,pos,0x99},

{36,10,00,30,pos,0x99},

{46,06,10,12,cir,0xff},

{56,13,00,22,pos,0x99},
{56,01,02,02,cir,0xff},
{.flag=0x82},
};

    seg_or_flag *orig_seg, *this_seg;

    this_seg = orig_seg = test_pat2;
    while(cycleCount-start_count <3000){
        //cursor_x=0;
        //cursor_y=0;

        if(ready!=0){  // otherwise wait until current_state==blanked
           uint8 int_status = CyEnterCriticalSection();
            
            preload_DAC_to_seg(this_seg,2,cursor_x,cursor_y);
            //CyDelayUs(12);

            AMux_1_Select(shape_to_mux[this_seg->seg_data.arc_type]);
            current_mask = this_seg->seg_data.mask;
            if(this_seg->seg_data.arc_type!=cir) current_mask=(current_mask ^ 0xff);
            times_to_loop = 2;
            ready=0;
            this_seg = successor(this_seg,orig_seg);
            current_state = start;
            
            
           CyExitCriticalSection(int_status);


}

    }    
    
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
    
    while(s[index++]) width+=char_width(*s)+4;    
    return mag*width;
}

void compileString(char *s, uint8 y_coord,uint8 buffer_index){  // turns a string into a display  list 
    uint8  mag=1;
    seg_or_flag *src_ptr;
    seg_or_flag *dst_ptr;
    int num_segs=0;     // so we don't overrun our fixed-size buffer
    
    int string_width = stringWidth(s,mag);
    uint8 x = 64 - (string_width / 2);    //center on 128 wide for now
    dst_ptr = seg_buffer[buffer_index];
    while(*s && num_segs<200){
        num_segs++;
        src_ptr = system_font[((uint8)*s)-32];
        while(src_ptr->seg_data.x_offset<0x80){
          dst_ptr->seg_data.x_offset = src_ptr->seg_data.x_offset+x;
          dst_ptr->seg_data.y_offset = src_ptr->seg_data.y_offset+y_coord;
          dst_ptr->seg_data.x_size = src_ptr->seg_data.x_size;
          dst_ptr->seg_data.y_size = src_ptr->seg_data.y_size;
          dst_ptr->seg_data.arc_type = src_ptr->seg_data.arc_type;
          dst_ptr->seg_data.mask = src_ptr->seg_data.mask;
          src_ptr++;
          dst_ptr++;
        }
        
        x += char_width(*s)+4;
        s++; 
    }
    dst_ptr->seg_data.x_offset = 0xff;       //used as a flag, but width not used
    dst_ptr->seg_data.mask=0;
}

void display_buffer(uint8 which_buffer){
    //long start_count = cycleCount;
    seg_or_flag *seg_ptr = seg_buffer[which_buffer];
    while(seg_ptr->seg_data.x_offset<0xff){

        if(ready){  // otherwise wait until current_state==blanked
           uint8 int_status = CyEnterCriticalSection();
            
            preload_DAC_to_seg(seg_ptr,2,0,0);
            //CyDelayUs(12);

            AMux_1_Select(shape_to_mux[seg_ptr->seg_data.arc_type]);
            current_mask = seg_ptr->seg_data.mask;
            if(seg_ptr->seg_data.arc_type!=cir) current_mask=(current_mask ^ 0xff);
            times_to_loop = 2;
            ready=0;
            seg_ptr++;
            //if(seg_ptr->seg_data.x_offset > 0xfe) seg_ptr = seg_buffer[which_buffer];
            current_state = start;
            
            
           CyExitCriticalSection(int_status);


        }
    }
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
 compileString("12:35 PM",0,0);
 compileString("April 1",50,1);
 compileString("Monday",100,2);
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
    long end_time = cycleCount;
    total_time = end_time-start_time;
    total_time +=1;
    
}
   

#define SCOPE_DELAY_SHORT 96
}

/* [] END OF FILE */
