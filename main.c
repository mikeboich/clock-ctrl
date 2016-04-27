/*******************************************************************************
 * FILE53: main.c
 *
 *
 * Description: 
 *  This is a source code for basic functionality of Vx1DAC8.
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
#include "font.h"
#include "max509.h"
#include <stdio.h>
#include <math.h>

// SPI constants:
#define TX_FIFO_NOT_FULL 4 

// Real time clock variables:
volatile int time_has_passed = 0;
int led_state = 0;  // we blink this once/second

// global screensaver offsets:
uint8 ss_x_offset=0, ss_y_offset=0;

#define BUF_ENTRIES 120
#define DEBUG_BUFFER 4
#define ANALOG_BUFFER 3
#define PONG_BUFFER 5
seg_or_flag seg_buffer[6][BUF_ENTRIES];

#define OVERWRITE 0
#define APPEND 1

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

typedef enum{textMode,analogMode, pongMode, debugMode} clock_type;
clock_type display_mode=textMode;

typedef enum{blank_unprimed,blank_primed,drawing}  draw_state;  // States of the draw loop/interrupt code

uint8 current_mask=0;
volatile draw_state current_state = blank_unprimed;
uint8 shape_to_mux[] = {2,0,1};
volatile int times_to_loop = 0;

volatile int cycle_count=0;  // poor man's timer
int last_refresh=0,loops_per_frame=0;

long total_time=0;

/* Define the start-of-segment interrupt routine 
   This routine loads the next 8 bit segment mask from the display list,
   and counts the number of drawing passes/segment.
*/

CY_ISR_PROTO(wave_started);

void wave_started(){
  isr_1_ClearPending();       // clear the interrupt
  cycle_count++;
  switch(current_state){

  case blank_unprimed:
    ShiftReg_1_WriteData(0x0);
    break;

  case blank_primed:
    current_state = drawing;
    break;
    
  case drawing:
    times_to_loop -= 1;
    if(times_to_loop==0){
      ShiftReg_1_WriteData(0x0);  // blank on the next cycle
      current_state = blank_unprimed;
    }
    else {
      ShiftReg_1_WriteData(current_mask);
    }
    break;
  }
}
    
uint8 pin(int x){
  if(x<0)x=0;
  if(x>255)x=255;
  return x;
}


void set_DACfor_seg(seg_or_flag *s,uint8 x, uint8 y){
  setImmediate(DAC_Reg_A | DAC_Pre_Load | s->seg_data.x_size);
  setImmediate(DAC_Reg_B | DAC_Pre_Load |s->seg_data.y_size);
  setImmediate(DAC_Reg_C | DAC_Pre_Load | (255-(s->seg_data.x_offset + x + ss_x_offset)));
  setImmediate(DAC_Reg_D | DAC_Pre_Load | (255-(s->seg_data.y_offset + y + ss_y_offset)));

}



// returns the width of a single vector character:
int char_width(char c){
  seg_or_flag *seg_ptr = system_font[((uint8) c)-32];    // map from char code to segment list
  while(seg_ptr->seg_data.x_offset<0x80) seg_ptr++;       // skip over segments
  return seg_ptr->flag & 0x7f;                 // end flag - 0x80 + char width
}

// returns the width (sum of character widths) of a string:
uint8 stringWidth(char s[],uint8 scale){
  int width=0,index=0;
    
  while(*s){
    width += char_width(*s);
    s++;
  }
  return pin(width*scale); 
}

// turns a string into a display  list:
// if the list is non-empty, it appends to the list.

void compileString(char *s, uint8 x_coord, uint8 y_coord,uint8 buffer_index,uint8 scale,int append){  
  seg_or_flag *src_ptr;
  seg_or_flag *dst_ptr;
  uint8 x;
  int num_segs=0;     // so we don't overrun our fixed-size buffer
    
  int kerning = (scale <= 2) ? 3: 2;
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

// test case data:
seg_or_flag test_segs[] = {
  {128,128,255,255,cir,0xff},
  {128,128,255,255,pos,0x99},
  {128,128,96,96,neg,0x99},
  {128,128,96,0,pos,0x99},
  {128,128,0,96,pos,0x99},
  {255,255,0,0,cir,0xff},
};

// compiles a list of segments into a display list.  Unlike CompileString, it doesn't modify them:
// if the list is non-empty, it appends to the list.

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

void line(uint8 x0, uint8 y0, uint8 x1, uint8 y1,int which_buffer){
  seg_or_flag the_line[] = {{0,0,0,0,pos,0x99},
			{.flag=0xff}};
  // We'd like to assume that x0 is the left-most point, so make it so:
  if(x0 > x1){
    uint8 tmp = x0;
    x0 = x1;
    x1 = tmp;
    
    tmp = y0;
    y0 = y1;
    y1 = tmp;
  }
  
  the_line->seg_data.x_offset = (x0 + x1) / 2;
  the_line->seg_data.y_offset = (y0 + y1) / 2;

  the_line->seg_data.x_size =  x1-x0;
  the_line->seg_data.y_size = (y1>y0) ? y1-y0:y0-y1;

  if(y1<y0){
    the_line->seg_data.arc_type = neg;
  }
 
  compileSegments(the_line,which_buffer, APPEND);
}
void line_test(){
 int i;
    for(i=5;i<250;i+=25){
        line(0,255-i,i,0,ANALOG_BUFFER);
        line(250,i,250-i,250,ANALOG_BUFFER);
    }
}

#define HR_HAND_WIDTH 8
#define HR_HAND_LENGTH 60
#define MIN_HAND_WIDTH 4
#define MIN_HAND_LENGTH 90
#define SEC_HAND_LENGTH 110

void drawClockHands(int h, int m, int s){
  if(h > 11) h -= 12;    // hours > 12 folded into 0-12  
  float hour_angle = (h/12.0) * M_PI * 2.0 + (m/60.0)*(M_PI/6.0);  // hour hand angle (we'll ignore the seconds)
  float minute_angle = (m/60.0) * M_PI*2.0 + (s/60.0)*(M_PI/30.0);  // minute hand angle
  float second_angle = (s/60.0)*M_PI*2.0;


  // not doing the 2-d hands yet, just lines
  line(128,128,128 + sin(hour_angle)*HR_HAND_LENGTH,128 + cos(hour_angle) * HR_HAND_LENGTH,ANALOG_BUFFER);  // draw the hour hand
  line(128,128,128 + sin(minute_angle)*MIN_HAND_LENGTH ,128 + cos(minute_angle) * MIN_HAND_LENGTH,ANALOG_BUFFER);
  line(128,128,128 + sin(second_angle)*SEC_HAND_LENGTH,128 + cos(second_angle) * SEC_HAND_LENGTH,ANALOG_BUFFER);
}

void updateAnalogClock(int hour, int min,int sec){
  int i;
  float angle=0.0;
  static char *nums[12] = {"12","1","2","3","4","5","6","7","8","9","10","11"};
  seg_or_flag face[] = {{128,128,255,255,cir,0xff},
			{128,128,8,8,cir,0xff},
			{.flag=0xff}};
  compileSegments(face,ANALOG_BUFFER,OVERWRITE);
  compileString("12",110,218,ANALOG_BUFFER,1,APPEND);
  compileString("6",118,16,ANALOG_BUFFER,1,APPEND);
  compileString("3",220,120,ANALOG_BUFFER,1,APPEND);
  compileString("9",20,120,ANALOG_BUFFER,1,APPEND);
    
  //    for(i=0;i<12;i++){
  //        uint8 x  = (uint8) (128.0+96.0*sin(angle)-8);
  //        uint8 y = (uint8) (128.0 + 96.0*cos(angle)-8);
  //        angle += (6.2830/12.0);
  //        compileString(nums[i],x,y,ANALOG_BUFFER,1,APPEND);
  //    }
  //    

  drawClockHands(hour,min,sec);
}

/* ************* Pong Game ************* */
#define PADDLE_HEIGHT 24
#define PADDLE_WIDTH   8
typedef struct{
    int paddle_position[2]; 
    int puck_velocity[2];
    int puck_position[2];
    int score[2];
  } pong_state;

pong_state game_state = {
    .paddle_position = {96,140},
    .puck_velocity = {2,1},
    .puck_position = {128,200},
    .score = {0,0}};

//returns which edge puck has struck, or zero otherwise:
// left = 1, right = 2, top = 3, bottom = 4
  int puck_at_edge(){
    if(game_state.puck_position[0] < PADDLE_WIDTH) return(1);
    if(game_state.puck_position[0]>255-PADDLE_WIDTH) return(2);
    if(game_state.puck_position[1] < 8) return(3);
    if(game_state.puck_position[1 ]>247) return(4);

    return(0);
       
}
#define PADDLE_MIN 16
#define PADDLE_MAX 238
void update_paddles(){
 int player;
    
    for(player=0;player<2;player++){
     if(game_state.paddle_position[player] > game_state.puck_position[1] && game_state.paddle_position[player] > PADDLE_MIN )
        game_state.paddle_position[player] -= 1;
     
        if (game_state.paddle_position[player] < game_state.puck_position[1] && \
                game_state.paddle_position[player] < PADDLE_MAX) game_state.paddle_position[player] += 1;
    }
}

// returns the new y velocity for the puck if it hit a paddle, and 0 otherwise
int puck_hit_paddle(){
    int which_paddle;
    int result=0;
    if(game_state.puck_position[0] < PADDLE_WIDTH)
      which_paddle = 0;
    else if(game_state.puck_position[0] > 254-PADDLE_WIDTH)
      which_paddle=1;
    else which_paddle = 2;
    
  
    switch(which_paddle){
        case 0: 
          result = game_state.puck_position[1]-game_state.paddle_position[0];
          break;
        
        case 1: 
          result = game_state.puck_position[1]-game_state.paddle_position[1];
          break;
        
    }
    
    if (result > PADDLE_HEIGHT/2) result=0;
    if (result < -PADDLE_HEIGHT/2) result=0;
    return result / 8;  
}

void pong_update(){
    int dim;
    
    for(dim=0;dim<2;dim++){
        game_state.puck_position[dim] += game_state.puck_velocity[dim];  // move the puck
    }
    update_paddles();
    int new_y_velocity = puck_hit_paddle();
    
    if(new_y_velocity){
        game_state.puck_velocity[1] = new_y_velocity;
        game_state.puck_velocity[0] = -game_state.puck_velocity[0];
    }
    int which_edge = puck_at_edge();
    if(which_edge  && !new_y_velocity){
        if(which_edge==1 || which_edge==2){
         // puck is exiting the playing area
            // just reverse for now:
          game_state.puck_velocity[0] = -game_state.puck_velocity[0];
        
        }
        if(which_edge == 2 || which_edge==4){  // hit top or bottom edge
         game_state.puck_velocity[1] = - game_state.puck_velocity[1]; 
        }
    }
    
}

    
void clear_buffer(int which_buffer){
    seg_buffer[which_buffer][0].seg_data.x_offset = 0xff;
}
void draw_pong(pong_state the_state){
    int x,y;
    
    clear_buffer(PONG_BUFFER);
    // draw the left paddle
    for(y=the_state.paddle_position[0]-(PADDLE_HEIGHT/2);y<the_state.paddle_position[0]+(PADDLE_HEIGHT/2)+1;y++) \
      line(0,y,PADDLE_WIDTH,y,PONG_BUFFER);
    // draw the right paddle:
    for(y=the_state.paddle_position[1]-(PADDLE_HEIGHT/2);y<the_state.paddle_position[1]+(PADDLE_HEIGHT/2)+1;y++) \
      line(254-PADDLE_WIDTH,y,254,y,PONG_BUFFER);
    
    // draw puck:
    x = the_state.puck_position[0];
    for(y=the_state.puck_position[1]-2;y<the_state.puck_position[1]+3;y++)
      line(x-2,y,x+2,y,PONG_BUFFER);
    
}
void display_buffer(uint8 which_buffer){
  //long start_count = cycle_count;
  seg_or_flag *seg_ptr = seg_buffer[which_buffer];
  while(seg_ptr->seg_data.x_offset != 0xff){

    if(current_state==blank_unprimed){  
      uint8 int_status = CyEnterCriticalSection();
            
      set_DACfor_seg(seg_ptr,0,0);
      //CyDelayUs(12);
      AMux_1_Select(shape_to_mux[seg_ptr->seg_data.arc_type]);
            
            
      times_to_loop = (seg_ptr->seg_data.x_size>seg_ptr->seg_data.y_size) ? \
	seg_ptr->seg_data.x_size/6 : seg_ptr->seg_data.y_size/6;
      if(times_to_loop==0) times_to_loop = 1;
      if(seg_ptr->seg_data.arc_type == cir) times_to_loop *= 2;  // circles don't double up like lines

            
      //times_to_loop = 4;
      // performance measurement:
      if(which_buffer != DEBUG_BUFFER) loops_per_frame+=times_to_loop+1;
            
            
            
      //int length = sqrt(seg_ptr->seg_data.x_size + seg_ptr->seg_data.y_size);
      //times_to_loop = length/32 + 1;

      current_mask = seg_ptr->seg_data.mask;
      if(seg_ptr->seg_data.arc_type!=cir) current_mask=(current_mask ^ 0xff);
      ShiftReg_1_WriteData(current_mask);

      current_state = blank_primed;
      strobe_LDAC();
      seg_ptr++;
          
      CyExitCriticalSection(int_status);


    }
  }
}

void initTime(){
  RTC_1_TIME_DATE *the_time = RTC_1_ReadTime();
  RTC_1_DisableInt();
    
  the_time->Month = 4;
  the_time->DayOfMonth = 26;
  the_time->DayOfWeek=3;
  the_time->Year = 2016;
  the_time->Hour = 17;
  the_time->Min = 51;
  the_time->Sec = 30;
    
  RTC_1_WriteTime(the_time);
  RTC_1_WriteIntervalMask(RTC_1_INTERVAL_SEC_MASK);
  RTC_1_EnableInt();
    
}

void updateTimeDisplay(){
  char time_string[32];
  char day_of_week_string[12];
  char date_string[15];
  char *day_names[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
  char *month_names[12] = {"Jan", "Feb", "Mar", "April","May","June","July","Aug","Sep","Oct","Nov","Dec"};

  RTC_1_TIME_DATE *the_time;
  the_time = RTC_1_ReadTime();
  int seconds = the_time->Sec;
  int minutes = the_time->Min;
  int hours = the_time->Hour;
    
  int day_of_week = the_time->DayOfWeek;
  int month = the_time->Month;
  int day_of_month = the_time->DayOfMonth;
  int year = the_time->Year;
    
  //update the interim screen-saver:
  ss_x_offset = (minutes) % 5;
  ss_y_offset =(minutes+2) % 5;
    
    
  sprintf(time_string,"%i:%02i:%02i",hours,minutes,seconds);
  compileString(time_string,255,0,0,3,0);

    
  //    sprintf(time_string,"%i:%02i",hours,minutes);
  //    compileString(time_string,0,0,4);

  sprintf(date_string,"%s %02i, %i",month_names[month-1],day_of_month,year);
  //sprintf(date_string,"April 15, 2016");
  compileString(date_string,255,114,1,1,OVERWRITE);
  // compileString("Hi Mom!",80,1,1);
    
  if(display_mode == analogMode || display_mode == debugMode) updateAnalogClock(hours,minutes,seconds);
    
  char dw[12];
  sprintf(dw,"%s",day_names[day_of_week-1]);
  compileString(dw,255,176,2,2,OVERWRITE);

}



void diagPattern(){
  system_font[0] = (seg_or_flag*)&TestCircle;
  compileString(" ",255,0,0,1,OVERWRITE);
  for(;;) display_buffer(0);
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
    
  //ShiftReg_1_WriteData(0xaa);  // not really needed..

  // Start the quad decoder:
  QuadDec_1_Start();

    
  /* Initialize Interrupt: */
  isr_1_StartEx(wave_started);
  CyGlobalIntEnable;

  //start the real-time clock component (which is what this is all about..)
  RTC_1_Start();
  LED_Reg_Write(1);
  initTime();
   

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

  //for(;;);
  //diagPattern();
  compileSegments(test_segs,0,OVERWRITE);

  uint8 cc = 0;
  // compileString("04/15/2016",0,0,1);
  compileString("4567",255,90,1,1,OVERWRITE);
  compileString("890",255,180,2,1,OVERWRITE);
  for(;;){
    //    int phase = SixtyHz_Read();
    //    while(SixtyHz_Read() == phase);   // wait for a 60Hz edge..
    
    if(display_mode == textMode){
      display_buffer(2);
      display_buffer(1);
      display_buffer(0);
    }
    else if(display_mode == analogMode){
      display_buffer(ANALOG_BUFFER);
    }
    else{
     if(display_mode == pongMode) display_buffer(PONG_BUFFER);   
    }
    pong_update();
    draw_pong(game_state);
    if(display_mode == debugMode){
      int elapsed = (cycle_count-last_refresh);
      char debug_str[32];
      sprintf(debug_str,"%i/%i/%i",31250/elapsed,elapsed,loops_per_frame);
      //sprintf(debug_str,"%i",31250/elapsed);
      compileString(debug_str,255,230,DEBUG_BUFFER,1,OVERWRITE);
      loops_per_frame=0;
      display_buffer(DEBUG_BUFFER);
      last_refresh = cycle_count;

    }
    if(time_has_passed){
      led_state = 1-led_state;
      LED_Reg_Write(led_state);
      updateTimeDisplay();
      time_has_passed = 0;     
    } 
    
    display_mode = QuadDec_1_GetCounter() % 4;
  }
   

#define SCOPE_DELAY_SHORT 96
}

/* [] END OF FILE */
