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
uint8 ss_x_offset=0, ss_y_offset=0;

// Routine to send data to the DAC over SPI.  Spins as necessary for full FIFO:
void setImmediate(uint16 spi_data){
  while((SPIM_1_ReadTxStatus() & TX_FIFO_NOT_FULL) == 0){   // spin if the fifo is full
  }
  SPIM_1_WriteTxData(spi_data);
}

// Drops the LDAC signal to load pre-buffered data in to the DAC:
// no longer used, since we write to the DAC during blanking periods
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
  setImmediate(DAC_Reg_A | DAC_Pre_Load | s->seg_data.x_size);
  setImmediate(DAC_Reg_B | DAC_Pre_Load |s->seg_data.y_size);
  setImmediate(DAC_Reg_C | DAC_Pre_Load | (255-(s->seg_data.x_offset + x + ss_x_offset)));
  setImmediate(DAC_Reg_D | DAC_Pre_Load | (255-(s->seg_data.y_offset + y + ss_y_offset)));

}
/* [] END OF FILE */
