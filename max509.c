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
#include "max509.h"
#include "font.h"
// SPI constants:
#define TX_FIFO_NOT_FULL 4 

// global screensaver offsets:
//uint8 ss_x_offset=0, ss_y_offset=0;

// Routine to send data to the DAC over SPI.  Spins as necessary for full FIFO:
void setImmediate(uint16 spi_data){
  while((SPIM_1_ReadTxStatus() & TX_FIFO_NOT_FULL) == 0){   // spin if the fifo is full
  }
  SPIM_1_WriteTxData(spi_data);
}

void strobe_LDAC(){
  LDAC_Write(0u);
  CyDelayUs(1);
  LDAC_Write(1u);
}

// pre-loads the DAC for the next segment.
// For reasons I haven't had time to understand,
// Things work better when I preload the DAC and then strobe LDAC
// a few instructions later, rather than doing Load immediate,
void set_DACfor_seg(seg_or_flag *s,uint8 x, uint8 y){
  //CyDelayUs(1);
  setImmediate(DAC_Reg_A | DAC_Pre_Load | s->seg_data.x_size);
  CyDelayUs(1);
  setImmediate(DAC_Reg_B | DAC_Pre_Load |s->seg_data.y_size);
  CyDelayUs(1);
  setImmediate(DAC_Reg_C | DAC_Pre_Load | (255-(s->seg_data.x_offset + x + x)));
  CyDelayUs(1);
  setImmediate(DAC_Reg_D | DAC_Pre_Load | (255-(s->seg_data.y_offset + y + y)));
 // CyDelayUs(1);

}

void bringup_test(){
    uint8 h[6] = {0,64,127,127,64,0};
    uint8 v[6] = {0,64,127,127,64,0};
    
    int i;
    
    
    for(i=0;i<6;i++){
        setImmediate(DAC_Reg_A | DAC_Pre_Load | h[i]);
        CyDelayUs(1);
        setImmediate(DAC_Reg_B | DAC_Pre_Load | v[i]);
        CyDelayUs(1);
        setImmediate(DAC_Reg_C | DAC_Pre_Load | h[i]);
        CyDelayUs(1);
        setImmediate(DAC_Reg_D | DAC_Pre_Load | v[i]);
        CyDelayUs(1);
        strobe_LDAC();
        CyDelay(1000);
    }
}
/* [] END OF FILE */
