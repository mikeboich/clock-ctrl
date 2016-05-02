/*  draw.c

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
#include "draw.h"

void clear_buffer(int which_buffer){
  seg_buffer[which_buffer][0].seg_data.x_offset = 0xff;
}

// turns a string into a display  buffer:
// if append !=0, it appends to the buffer
// otherwise it overwrites:

void compileString(char *s, uint8 x_coord, uint8 y_coord,uint8 buffer_index,uint8 scale,int append){  
  seg_or_flag *src_ptr;
  seg_or_flag *dst_ptr;
  uint8 x;
  int num_segs=0;     // so we don't overrun our fixed-size buffer
    
  int kerning = (scale <= 2) ? 4: 3;
  if(strlen(s)<6) kerning += 2;
  int string_width = stringWidth(s,scale) + (strlen(s)-1)*kerning;;
  if(x_coord==255){
    x_coord = pin(128 - (string_width / 2));    //center on 128 if x coord has magic value
  }
  dst_ptr = seg_buffer[buffer_index];
  while(append && dst_ptr->flag != 255){            // skip over existing entries;
    dst_ptr++;
    num_segs++;
  }
  while(*s && num_segs<BUF_ENTRIES){
    num_segs++;
    src_ptr = system_font[((uint8) *s)-32];
    while(src_ptr->seg_data.x_offset<0x80){
      dst_ptr->seg_data.x_offset = pin(scale*src_ptr->seg_data.x_offset+x_coord);
      dst_ptr->seg_data.y_offset = pin(scale*src_ptr->seg_data.y_offset+y_coord);
      dst_ptr->seg_data.x_size = pin(scale*src_ptr->seg_data.x_size);
      dst_ptr->seg_data.y_size = pin(scale*src_ptr->seg_data.y_size);
      dst_ptr->seg_data.arc_type = src_ptr->seg_data.arc_type;
      dst_ptr->seg_data.mask = src_ptr->seg_data.mask;
      src_ptr++;
      dst_ptr++;
    }
        
    x_coord = pin(x_coord + scale*char_width(*s) + kerning);
    s++; 
  }
  dst_ptr->seg_data.x_offset = 0xff;       //used as a flag, but width not used
  dst_ptr->seg_data.mask=0;
}

// compiles a list of segments into a display list.  Unlike CompileString, it doesn't modify them:
// if append !=0, it appends to the buffer.  Otherwise ir overwrites the buffer:

void compileSegments(seg_or_flag *src_ptr, uint8 buffer_index,int append){   
  seg_or_flag *dst_ptr;
  int num_segs=0;     // so we don't overrun our fixed-size buffer
    
  dst_ptr = seg_buffer[buffer_index];
  while(append && dst_ptr->flag != 255) {
    dst_ptr++;
    num_segs++;
  }
  while(src_ptr->seg_data.x_offset != 255 && num_segs<BUF_ENTRIES){
    num_segs++;
    dst_ptr->seg_data = src_ptr->seg_data;
    src_ptr++;
    dst_ptr++;
  }
  dst_ptr->seg_data.x_offset = 0xff;       // sentinel value
  dst_ptr->seg_data.mask=0;
}

/* [] END OF FILE */
