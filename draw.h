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
#ifndef draw_h
#define draw_h

#include "font.h"
    
// constants plus buffers to hold drawlists:
#define BUF_ENTRIES 120
#define DEBUG_BUFFER 4
#define ANALOG_BUFFER 3
#define PONG_BUFFER 5
    
#define OVERWRITE 0
#define APPEND 1

extern seg_or_flag seg_buffer[6][BUF_ENTRIES];

struct menu;  // "forward" definition of menu is fine for this purpose

void compileString(char *s, uint8 x_coord, uint8 y_coord,uint8 buffer_index,uint8 scale,int append);
void compileSegments(seg_or_flag *src_ptr, uint8 buffer_index,int append);
void compileMenu(struct menu* the_menu, uint8 buffer_index,int append);

#endif
/* [] END OF FILE */
