
/*  QuadDec.c

 Copyright (C) 2016 Michael Boich

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 I had to stop using the Cypress Quadrature Decoder, as I've run out
 of UDBs.  So this is a software-only replacement
 *******************************************************************************/
#include "QuadDec.h"
volatile int encoder_value = 0;

 static int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
 static int oldAB = 0;

CY_ISR_PROTO (Quad_A_isr);
CY_ISR_PROTO(Quad_B_isr);

void Quad_A_isr(){
    Quad_A_isr_ClearPending();
    uint8 ab = (QuadB_Read() << 1) | QuadA_Read();  // nicer to do in parallel but i need to learn how
    oldAB = (oldAB << 2) & 0xF;
    oldAB |= (ab & 0x3);
    encoder_value += enc_states[oldAB];
}


int QuadDec_Read(){
    return(encoder_value/4);
}

void QuadDec_Write(int value){
    encoder_value = value;
}

void QuadDec_Init(){
     Quad_A_isr_StartEx(Quad_A_isr);
 }

/* [] END OF FILE */
