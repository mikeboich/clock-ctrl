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


// SPI constants:
#define TX_FIFO_NOT_FULL 4 

#include "font.h"
#include "max509.h"

int test_index=0;

int knob_position;
void setImmediate(uint16 spi_data){
  // spin if the fifo is full:
  while((SPIM_1_ReadTxStatus() & TX_FIFO_NOT_FULL) == 0){
  }
  SPIM_1_WriteTxData(spi_data);

}


/* Define the start-of-segment interrupt routine 
   This routine loads the next 8 bit segment mask from the display list,
   and will eventually program the DACS for xCenter, yCenter, xRadius, and yRadius as well.
*/


void strobe_LDAC(){
    LDAC_Write(0u);
    LDAC_Write(1u);
}
seg_or_flag *this_seg = SpaceChar;


typedef enum{start, looping, blanked}  draw_state;  // States of the draw loop/interrupt code

uint8 current_mask;
volatile draw_state current_state = blanked;
uint8 cursor_x=0,cursor_y=0;
uint8 shape_to_mux[] = {2,0,1};
volatile int times_to_loop = 0;
volatile int ready=1;

volatile long cycleCount=0;  // poor man's timer

CY_ISR_PROTO(wave_started);

void wave_started(){
  isr_1_ClearPending();       // clear the interrupt
  cycleCount++;
#undef ztest
#ifndef ztest
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
#else
    ShiftReg_1_WriteData(current_mask);
#endif
}
    
  


  void preload_DAC_to_seg(seg_or_flag *s,int magnify){
    setImmediate(DAC_Reg_A | DAC_Pre_Load | magnify*s->seg_data.x_size);
    setImmediate(DAC_Reg_B | DAC_Pre_Load | magnify*s->seg_data.y_size);
    setImmediate(DAC_Reg_C | DAC_Pre_Load | (255-magnify*(s->seg_data.x_offset + cursor_x)));
    setImmediate(DAC_Reg_D | DAC_Pre_Load | (255-magnify*(s->seg_data.y_offset + cursor_y)));

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

void diag_test(uint8 char_code){
    long start_count = cycleCount;
   
   vector_font test_pat= {
    {64,64,128,128,cir,0xff},
    {00,00,32,32,cir,0xff},
    {00,127,32,32,cir,0xff},
    {127,0,32,32,cir,0xff},
    {127,127,32,32,cir,0xff},
    {.flag=0x88}

};
    seg_or_flag *origSeg;

    this_seg = origSeg = system_font[char_code] ;
    //this_seg = origSeg = test_pat;
    while(cycleCount-start_count < 12000){
        //cursor_x=0;
        //cursor_y=0;

        if(ready!=0){  // otherwise wait until current_state==blanked
           uint8 int_status = CyEnterCriticalSection();
            
            preload_DAC_to_seg(this_seg,1);
            //CyDelayUs(12);

            AMux_1_Select(shape_to_mux[this_seg->seg_data.arc_type]);
            current_mask = this_seg->seg_data.mask;
            if(this_seg->seg_data.arc_type!=cir) current_mask=(current_mask ^ 0xff);
            times_to_loop = 4;
            ready=0;
            this_seg = successor(this_seg,origSeg);
            current_state = start;
            
            
           CyExitCriticalSection(int_status);


}

    }    
    
}

void circle_test(){
    vector_font test_circle ={
    {10,10,0x80,0x80,cir,0x01},
    //{10,10,0x80,0x80,cir,0x02},
    //{10,10,0x80,0x80,cir,0x4},
    //{10,10,0x80,0x80,cir,0x08},
   // {10,10,0x80,0x80,cir,0x10},
   // {10,10,0x80,0x80,cir,0x20},
   // {10,10,0x80,0x80,cir,0x40},
    {10,10,0x80,0x80,cir,0x80},
    {.flag=0x82}};
    seg_or_flag *origSeg;
    
    this_seg = origSeg = test_circle ;
    for(;;){
        cursor_x=0;
        cursor_y=0;

        if(ready!=0){  // otherwise wait until current_state==blanked
           uint8 int_status = CyEnterCriticalSection();
            
            preload_DAC_to_seg(this_seg,1);
            //CyDelayUs(12);
            strobe_LDAC();

            AMux_1_Select(shape_to_mux[this_seg->seg_data.arc_type]);
            current_mask = this_seg->seg_data.mask;
            times_to_loop = 1;
            ready=0;
            this_seg = successor(this_seg,origSeg);
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

#ifndef ztest
 uint8 cc = 0;
 for(;;){
     diag_test(cc);
    //diag_test(QuadDec_1_GetCounter() % 104);
     cc = (cc + 1);
    if(cc>104) cc=0;
}
   
  circle_test();
#endif

#undef ANIMATE
#define SCOPE_DELAY_SHORT 96
#ifdef ztest
    for(;;){
        uint8 cursorx=0;
        uint8 cursory=0;
        uint8 savedx;
        vector_font *f;
        int i,j;
        for(i=0;i<105;i++){
            for(j=0;j<30;j++){
                drawLetter(i,cursorx,cursory);
            }
        }
        

    }
#endif
#ifdef ANIMATE
  for(;;) show_test_pattern();

int shape=0;
int x_points[] = {0,128,255};
int x,y;
for(;;){
    for(x=0;x<3;x++){
        for(y=0;y<3;y++){
            setRadius(64,64);
            setOrigin(x_points[x],x_points[y]);
            CyDelayUs(SCOPE_DELAY_SHORT);
        }
    }
    bigCircle();
    //CyDelay(3);
    frameDisplay();
    //CyDelay(3);
    //CyDelayUs(SCOPE_DELAY_SHORT);
    AMux_1_Select(shape);
    shape = (shape+1)%3;
}
#endif
#ifndef ztest


#endif
}


/* [] END OF FILE */
