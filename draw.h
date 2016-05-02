/*  draw.h

 Copyright (C) 2016 Michael Boich

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 Routines to compile strings, menus, and other primitives into display lists

 *******************************************************************************/
#ifndef draw_h
#define draw_h

#include "font.h"
    
// constants plus buffers to hold drawlists:
#define BUF_ENTRIES 120
#define DEBUG_BUFFER 4
#define ANALOG_BUFFER 3
#define PONG_BUFFER 5
#define MENU_BUFFER 0
    
#define OVERWRITE 0
#define APPEND 1

extern seg_or_flag seg_buffer[6][BUF_ENTRIES];

struct menu;  // "forward" definition of menu is fine for this purpose

void clear_buffer(int which_buffer);
void compileString(char *s, uint8 x_coord, uint8 y_coord,uint8 buffer_index,uint8 scale,int append);
void compileSegments(seg_or_flag *src_ptr, uint8 buffer_index,int append);
void compileMenu(struct menu* the_menu, uint8 buffer_index,int append);
void circle(uint8 x0, uint8 y0, uint8 radius,int which_buffer);
void line(uint8 x0, uint8 y0, uint8 x1, uint8 y1,int which_buffer);

#endif
/* [] END OF FILE */
