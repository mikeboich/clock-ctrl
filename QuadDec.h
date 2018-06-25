/*  QuadDec.h

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
#include <device.h>

int QuadDec_Read();
void QuadDec_Write(int value);
void QuadDec_Init();


/* [] END OF FILE */
