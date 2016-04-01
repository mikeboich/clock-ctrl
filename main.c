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


/* Define the end-of-arc interrupt routine 
   This routine loads the next 8 bit segment mask from the display list,
   and will eventually program the DACS for xCenter, yCenter, xRadius, and yRadius as well.
*/
CY_ISR_PROTO(wave_finished);

void wave_finished(){
  static uint8 mask[8] = {255,127,63,31,15,7,3,0};  //test data for now
  static uint8 index = 0;
    
  isr_1_ClearPending();       // clear the interrupt
  ShiftReg_1_WriteData(mask[index]);  // and pre-load the next mask
 
  index = (index + 1) % 8;
}

void rings(){
  int v1 = 240;
  int v2 = 120;
  int i;
        
  for(i=0;i<4;i++){
    SPIM_1_WriteTxData(DAC_Reg_B | DAC_Pre_Load | v1);
    SPIM_1_WriteTxData(DAC_Reg_A | DAC_Pre_Load | v1);
    ldac_pin_Write(0u);
    ldac_pin_Write(1u);
    CyDelay(100);
  }

  for(i=0;i<4;i++){
    SPIM_1_WriteTxData(DAC_Reg_B | DAC_Pre_Load | v2);
    SPIM_1_WriteTxData(DAC_Reg_A | DAC_Pre_Load | v2);
    ldac_pin_Write(0u);
    CyDelayUs(1);
    ldac_pin_Write(1u);
    CyDelay(100);
  }

   
}

void wiggle(){
  int amplitudes[8] = {2,4,8,16,32,64,128,255};
  int i;
  uint8 status_bits = 0;
  //get tall:
  for(i=0;i<255;i+=8){
    SPIM_1_WriteTxData(DAC_Reg_A | DAC_Pre_Load  | i);
    while((SPIM_1_ReadTxStatus() & TX_FIFO_NOT_FULL) == 0){
    }
    SPIM_1_WriteTxData(DAC_Reg_B | DAC_Pre_Load  | i);
    while((SPIM_1_ReadTxStatus() & TX_FIFO_NOT_FULL) == 0){
    }
    //CyDelayUs(2);
    LDAC_Write(0u);
    CyDelayUs(1);
    LDAC_Write(1u);
    CyDelayUs(10000);
  }

}
void setImmediate(uint16 spi_data){
  // spin if the fifo is full:
  while((SPIM_1_ReadTxStatus() & TX_FIFO_NOT_FULL) == 0){
  }
  SPIM_1_WriteTxData(spi_data);

}

void setOrigin(uint8 x,uint8 y){
    setImmediate(DAC_Reg_C | DAC_Load_Now |  x);
    setImmediate(DAC_Reg_D | DAC_Load_Now |  y);
}

void setRadius(uint8 x,uint8 y){
    setImmediate(DAC_Reg_A | DAC_Load_Now |  x);
    setImmediate(DAC_Reg_B | DAC_Load_Now |  y);
}
void frameDisplay(){
    AMux_1_Select(0);
    setRadius(255,0);
    setOrigin(128,0);
    CyDelayUs(128);
    setOrigin(128,255);
    CyDelayUs(128); 
    
    setRadius(0,255);
    setOrigin(0,128);
    CyDelayUs(128);
    setOrigin(255,128);
    CyDelayUs(128);
}

void bigCircle(){
    AMux_1_Select(1);
    setRadius(128,128);
    setOrigin(128,128);
    CyDelayUs(128);
    
}

void bounce(){
  static uint8 grid_points[] = {32,223};
  static uint8 radii[] = {64,80,96,112,128,144};
  static int x=0,y=0,r=3;
  static int xincr=2;
  static int yincr=1;
  static int rincr = 1;
    
  for(;;){
    setImmediate(DAC_Reg_C | DAC_Load_Now |  x);
    setImmediate(DAC_Reg_D | DAC_Load_Now |  y); 
    setImmediate(DAC_Reg_A | DAC_Load_Now | r);
    setImmediate(DAC_Reg_B | DAC_Load_Now | r);

    CyDelay(16);
    x = x+xincr;
    y = y+ yincr;
    
    if(x>=253 || x<2) xincr = -xincr;;
    if(y>=254 || y<1) {
      yincr = -yincr;
    }


    r += rincr;
    if(r>127 || r<1) rincr = -rincr;

  }

  //whichWave = (whichWave+1) % 3;
  //CyDelayUs(32);  
   
    
}

void draw_seg(vc_segment s,uint8 x, uint8 y){
    static uint8 seg_to_seg[] = {2,0,1};
    setImmediate(DAC_Reg_A | DAC_Pre_Load | s.x_size);
    setImmediate(DAC_Reg_B | DAC_Pre_Load | s.y_size);
    setImmediate(DAC_Reg_C | DAC_Pre_Load | 255-(s.x_offset + x));
    setImmediate(DAC_Reg_D | DAC_Pre_Load | 255-(s.y_offset + y));
    while((SPIM_1_ReadTxStatus() & TX_FIFO_NOT_FULL) == 0){
    }
    AMux_1_Select(seg_to_seg[(uint8) s.arc_type]);
    LDAC_Write(0u);
    LDAC_Write(1u);
    CyDelay(16);
}


void drawVectorChar(vector_font ch,uint8 *x, uint8 *y){
    seg_or_flag *cp=ch;
    
        while(cp->seg_data.x_offset < 0x80){
        draw_seg(cp->seg_data,*x,*y);
        cp++;
    }
    *x += (cp->flag & 0x7f);

    
}

int drawLetter(uint8 i,uint8 x, uint8 y){
    seg_or_flag *char_info = system_font[i];
    
     while(char_info->seg_data.x_offset < 0x80){
        draw_seg(char_info->seg_data,x,y);
        char_info++;
    }
    return(x + (char_info->flag & 0x7f));       
}

void show_test_pattern(){
    frameDisplay();
    CyDelay(16);
    bigCircle();
    CyDelay(16);
    drawLetter(36,192,64);
    CyDelay(16);
    drawLetter(36,192,64);
    CyDelay(16);
    drawLetter(37,64,192);
    CyDelay(16);
    drawLetter(38,192,192);
    
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
    
  ShiftReg_1_WriteData(0xaa);  // not really needed..

// Start the quad decoder:
   QuadDec_1_Start();
    
  /* Initialize Interrupt: */
  isr_1_StartEx(wave_finished);
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

#undef ANIMATE
#define SCOPE_DELAY_SHORT 96
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

    for(;;){
        uint8 cursorx=QuadDec_1_GetCounter()+128;
        uint8 cursory=128;
        uint8 savedx;
        vector_font *f;
        int i,j;
        for(i=0;i<105;i++){
            for(j=0;j<10;j++){
                cursorx=cursory=QuadDec_1_GetCounter()+128;
                drawLetter(i,cursorx,cursory);
            }
        }
        

    }

}


/* [] END OF FILE */
