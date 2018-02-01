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
#define BUF_ENTRIES 300
#define DEBUG_BUFFER 1
#define MAIN_BUFFER 0
#define AUX_BUFFER 2
    
#define OVERWRITE 0
#define APPEND 1

extern seg_or_flag seg_buffer[3][BUF_ENTRIES];

struct menu;  // "forward" definition of menu is fine for this purpose

void clear_buffer(int which_buffer);
void compileString(char *s, uint8 x_coord, uint8 y_coord,uint8 buffer_index,uint8 scale,int append);
void compile_substring(char *s, uint8 count,uint8 x_coord, uint8 y_coord,uint8 which_buffer,uint8 scale,uint8 append);
void compileSegments(seg_or_flag *src_ptr, uint8 buffer_index,int append);
void offsetSegments(seg_or_flag *src_pre, uint8_t x, uint8_t y);
void insetSegments(seg_or_flag *src_pre, uint8_t x, uint8_t y);
void compileMenu(struct menu* the_menu, uint8 buffer_index,int append);
void circle(uint8 x0, uint8 y0, uint8 radius,int which_buffer);
void line(uint8 x0, uint8 y0, uint8 x1, uint8 y1,int which_buffer);
void vertical_dashed_line(uint8 x0, uint8 y0, uint8 x1, uint8 y1,int which_buffer);

// some utilities for animation:
void copyBuf(int src_buffer,int dst_buffer);

// adding a debug macro here for now.  Maybe we'll create a new place for it in the future:
char msg_buf[255];


#endif
/* [] END OF FILE */
