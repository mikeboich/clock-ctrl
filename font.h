/* ========================================
 *
 * Font code for my Oscilloclock derivative
 * Copyright Michael Boich, 2016
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * Based on the excellent work of Aaron Stokes and David Forbes
 * ========================================
*/
#ifndef font_h
#define font_h
#include <device.h>
/* Define the vector font types:

    Each character is a series of segments, and each segment contains the following:
        XOffset     (offset from lower left box in which character is being rendered)
        YOffset     (likewise)
        XSize       magnitude of sinusoid in X
        YSize       likewise
        shape       0 for sin, -sin, 1 for sin, sin, and 2 for sin, cos
        blanking    8 bit mask for blanking (or not) each of the 8 segments of the cir
        

// I set the PSOC analog mux up so that mux0 = /, 1 = circle, and 2 = \, so some translating is required
typedef enum  shape {neg,circle,pos} shape_type;

*/

typedef enum {neg,pos,cir} shape;  

typedef  struct {
    uint8 x_offset;
    uint8 y_offset;
    uint8 x_size;
    uint8 y_size;
    shape arc_type;
    uint8 mask;
} vc_segment;

typedef  union{
        vc_segment seg_data;
        uint8   flag;
} seg_or_flag;

typedef seg_or_flag vector_font[];

//const int kerning=4;

extern seg_or_flag *system_font[128];

void init_font();

#endif

/* [] END OF FILE */
