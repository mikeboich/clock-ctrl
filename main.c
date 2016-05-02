/*  main.c

 Copyright (C) 2016 Michael Boich

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 A re-creation of the David Forbes  vector-graphics clock, implemented on Cypress PSOC
 and coded in C.

 *******************************************************************************/

#include <device.h>
#include "font.h"
#include "draw.h"
#include "max509.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// SPI constants:
#define TX_FIFO_NOT_FULL 4 

// Real time clock variables:
volatile int time_has_passed = 0;
int led_state = 0;  // we blink this once/second

// encoder button state
//int button_changed=0;  // not currently used, as we're not using interrupts yet
int button_state=0;

// global screensaver offsets:
uint8 ss_x_offset=0, ss_y_offset=0;

#define BUF_ENTRIES 120
#define DEBUG_BUFFER 4
#define ANALOG_BUFFER 3
#define PONG_BUFFER 5
seg_or_flag seg_buffer[6][BUF_ENTRIES];

#define OVERWRITE 0
#define APPEND 1

// Some useful strings:
char *day_names[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
char *month_names[12] = {"Jan", "Feb", "Mar", "April","May","June","July","Aug","Sep","Oct","Nov","Dec"};


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

typedef enum{textMode,analogMode, pongMode,pendulumMode} clock_type;
clock_type display_mode=textMode;

int verbose_mode = 0;

typedef enum{blank_unprimed,blank_primed,drawing}  draw_state;  // States of the draw loop/interrupt code

uint8 current_mask=0;
volatile draw_state current_state = blank_unprimed;
uint8 shape_to_mux[] = {2,0,1};
volatile int times_to_loop = 0;

volatile int cycle_count=0;  // poor man's timer
int error_term=0;            // difference between hw counters and 1 pps edge

int last_refresh=0,loops_per_frame=0;

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


void set_DACfor_seg(seg_or_flag *s,uint8 x, uint8 y){
  setImmediate(DAC_Reg_A | DAC_Pre_Load | s->seg_data.x_size);
  setImmediate(DAC_Reg_B | DAC_Pre_Load |s->seg_data.y_size);
  setImmediate(DAC_Reg_C | DAC_Pre_Load | (255-(s->seg_data.x_offset + x + ss_x_offset)));
  setImmediate(DAC_Reg_D | DAC_Pre_Load | (255-(s->seg_data.y_offset + y + ss_y_offset)));

}

// test case data:
seg_or_flag test_segs[] = {
  {128,128,255,255,cir,0xff},
  {128,128,96,96,neg,0x99},
  {128,128,96,0,pos,0x99},
  {128,128,0,96,pos,0x99},
  {255,255,0,0,cir,0xff},
};


void clear_buffer(int which_buffer){
  seg_buffer[which_buffer][0].seg_data.x_offset = 0xff;
}

void circle(uint8 x0, uint8 y0, uint8 radius,int which_buffer){
  seg_or_flag the_circle[] = {{0,0,0,0,cir,0xff},
			      {.flag=0xff}};
  the_circle->seg_data.x_offset = x0;
  the_circle->seg_data.y_offset = y0;

  the_circle->seg_data.x_size =  radius;
  the_circle->seg_data.y_size = radius;
 
  compileSegments(the_circle,which_buffer, APPEND);
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
#define MIN_HAND_LENGTH 100
#define SEC_HAND_LENGTH 124

void drawClockHands(int h, int m, int s){
  if(h > 11) h -= 12;    // hours > 12 folded into 0-12  
  float hour_angle = (h/12.0) * M_PI * 2.0 + (m/60.0)*(M_PI/6.0);  // hour hand angle (we'll ignore the seconds)
  float minute_angle = (m/60.0) * M_PI*2.0 + (s/60.0)*(M_PI/30.0);  // minute hand angle
  float second_angle = ((s/60.0))*M_PI*2.0;


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
    
  drawClockHands(hour,min,sec);
  //experimental one revoultion/second widget:
  float x = 128.0 + (SEC_HAND_LENGTH-4)*sin(2*M_PI*(cycle_count-error_term)/31250.0);
  float y = 128.0 + (SEC_HAND_LENGTH-4)*cos(2*M_PI*(cycle_count-error_term)/31250.0);
  circle(x,y,16,ANALOG_BUFFER);
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
  .puck_velocity = {4,2},
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
int puck_visible(){
  if(game_state.puck_position[0] > 0 || game_state.puck_position[0]<250)
    return 1;
  else 
    return 0;
}
#define PADDLE_MIN 16
#define PADDLE_MAX 238
void update_paddles(){
  int player;
  RTC_1_TIME_DATE *the_time;
  the_time = RTC_1_ReadTime();
  int should_miss[2] = {0,0}; //set to 1 if we want that player to miss
  
  if(puck_visible()){
    if((the_time->Min ==59 && the_time->Sec>57) || game_state.puck_velocity[0]<0) should_miss[1]=1;
    if((the_time->Sec > 57) || game_state.puck_velocity[0]>0) should_miss[0]=1;

    for(player=0;player<2;player++){
      if(!should_miss[player]){
	if(game_state.paddle_position[player] > game_state.puck_position[1] && game_state.paddle_position[player] > PADDLE_MIN)
	  game_state.paddle_position[player] -= 4;
         
	if (game_state.paddle_position[player] < game_state.puck_position[1] && \
	    game_state.paddle_position[player] < PADDLE_MAX) game_state.paddle_position[player] += 4;

      }
    }
  }

}

// returns the new y velocity for the puck if it hit a paddle, and 0 otherwise
int puck_hit_paddle(){
  int which_paddle;
  int result=0;
  if(game_state.puck_position[0] < PADDLE_WIDTH && game_state.puck_position[0] - game_state.puck_velocity[0]>PADDLE_WIDTH)
    which_paddle = 0;
  else if(game_state.puck_position[0] > 254-PADDLE_WIDTH  && game_state.puck_position[0] - game_state.puck_velocity[0]<= 254-PADDLE_WIDTH)
    which_paddle=1;
  else return 0;
  
  result = game_state.puck_position[1]-game_state.paddle_position[which_paddle];
        
  if(abs(result) > PADDLE_HEIGHT/2) result=0;  // we missed

  return result / 8;  
}

void pong_update(){
  int dim;
    
  for(dim=0;dim<2;dim++){
    game_state.puck_position[dim] += game_state.puck_velocity[dim];  // move the puck
  }
    
  // if puck has been off screen, and is beyond some imaginary boundary, recenter it:
  if(game_state.puck_position[0] < -128 || game_state.puck_position[0]>384){
    game_state.puck_position[0] = 128;   
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
    if(which_edge == 3 || which_edge==4){  // hit top or bottom edge. reverse y velocity:
      game_state.puck_velocity[1] = -game_state.puck_velocity[1]; 
    }
  }
    
}

void render_pong_buffer(pong_state the_state){
  int x,y;
  RTC_1_TIME_DATE *the_time;
    
  the_time = RTC_1_ReadTime();
    
  clear_buffer(PONG_BUFFER);
  // draw the left paddle
  for(y=the_state.paddle_position[0]-(PADDLE_HEIGHT/2);y<the_state.paddle_position[0]+(PADDLE_HEIGHT/2)+1;y++) \
    line(0,y,PADDLE_WIDTH,y,PONG_BUFFER);
  // draw the right paddle:
  for(y=the_state.paddle_position[1]-(PADDLE_HEIGHT/2);y<the_state.paddle_position[1]+(PADDLE_HEIGHT/2)+1;y++) \
    line(254-PADDLE_WIDTH,y,254,y,PONG_BUFFER);
    
  // draw puck:
  x = the_state.puck_position[0];
  if(puck_visible()){
    for(y=the_state.puck_position[1]-2;y<the_state.puck_position[1]+3;y++)
      line(x-2,y,x+2,y,PONG_BUFFER);
  }
    
  // draw the centerline:
  x=128;
  for(y=240;y>0;y-=32){
    line(128,y,128,y-16,PONG_BUFFER);   
  }
    
  // draw the hours and minutes as two scores:
  char time_str[32];
  int the_hour = the_time->Hour;
  int the_minute = the_time->Min;
  sprintf(time_str,"%02i",the_hour);
  compileString(time_str,32,210,PONG_BUFFER,2,APPEND);
    
  sprintf(time_str,"%02i",the_minute);
  compileString(time_str,174,210,PONG_BUFFER,2,APPEND);
    
}

/*  Pendulum Clock *** */
void render_pendulum_buffer(){
  RTC_1_TIME_DATE *the_time;
  char sec_str[32],hr_min_string[32];
  float x,y,i;
  the_time = RTC_1_ReadTime();
  sprintf(sec_str,"%02i",the_time->Sec);
  compileString(sec_str,255,32,PONG_BUFFER,2,OVERWRITE);

  sprintf(hr_min_string,"%02i:%02i",the_time->Hour,the_time->Min);
  compileString(hr_min_string,255,130,PONG_BUFFER,3,APPEND);

  x = 128.0+200*sin(sin(2*M_PI*(cycle_count-error_term)/31250.0)/2.5);
  y = 250.0 - 200*cos(sin(2*M_PI*(cycle_count-error_term)/31250.0)/2.5);
  line(128,250,x,y,PONG_BUFFER);
  for(i=32;i>0;i-=8) circle(x,y,i,PONG_BUFFER);
  circle(128,250,8,PONG_BUFFER);

}


void display_buffer(uint8 which_buffer){
  //long start_count = cycle_count;
  seg_or_flag *seg_ptr = seg_buffer[which_buffer];
  while(seg_ptr->seg_data.x_offset != 0xff){

    if(current_state==blank_unprimed){  
      uint8 int_status = CyEnterCriticalSection();
            
      set_DACfor_seg(seg_ptr,0,0);
      AMux_1_Select(shape_to_mux[seg_ptr->seg_data.arc_type]);
              
      times_to_loop = (seg_ptr->seg_data.x_size>seg_ptr->seg_data.y_size) ? \
	seg_ptr->seg_data.x_size/6 : seg_ptr->seg_data.y_size/6;
      if(times_to_loop==0) times_to_loop = 1;
      if(seg_ptr->seg_data.arc_type == cir) times_to_loop *= 2;  // circles don't double up like lines

     
      // performance measurement:
      if(which_buffer != DEBUG_BUFFER) loops_per_frame+=times_to_loop+1;
            
      current_mask = seg_ptr->seg_data.mask;
      if(seg_ptr->seg_data.arc_type!=cir) current_mask=(current_mask ^ 0xff);  // I must have wired something wrong for this to be needed!
      ShiftReg_1_WriteData(current_mask);  // "prime" the shift register

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
    
  the_time->Month = 5;
  the_time->DayOfMonth = 2;
  the_time->DayOfWeek=2;
  the_time->Year = 2016;
  the_time->Hour = 10;
  the_time->Min = 21;
  the_time->Sec = 30;
    
  RTC_1_WriteTime(the_time);
  RTC_1_WriteIntervalMask(RTC_1_INTERVAL_SEC_MASK);
  RTC_1_EnableInt();
    
}

void updateTimeDisplay(){
  char time_string[32];
  char day_of_week_string[12];
  char date_string[15];

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

  sprintf(date_string,"%s %i, %i",month_names[month-1],day_of_month,year);
 
  compileString(date_string,255,114,1,1,OVERWRITE);
     
  char dw[12];
  sprintf(dw,"%s",day_names[day_of_week-1]);
  compileString(dw,255,176,2,2,OVERWRITE);


}

#define BUTTON_DOWN 0
#define BUTTON_UP 1
void poll_button(){
  static int last_update = 0;
  int tmp = EncoderButton_Read();
    
  if(tmp != button_state && cycle_count-last_update > 300){
    button_state = tmp;
    last_update = cycle_count;
    if(button_state == 0) verbose_mode = 1-verbose_mode;
  }
}


void set_time(){
  static int hours, minutes, seconds, day, month, year, day_of_week;
    
    
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

  // Initialize button interrupt:
  //button_isr_Start();
    
  /* Initialize Wave Interrupt: */
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

  compileSegments(test_segs,0,OVERWRITE);

  uint8 cc = 0;

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
        RTC_1_TIME_DATE *now = RTC_1_ReadTime();
      if(display_mode == analogMode) updateAnalogClock(now->Hour,now->Min,now->Sec);
      display_buffer(ANALOG_BUFFER);
    }
    else{
      if(display_mode == pongMode){
    	display_buffer(PONG_BUFFER);
        pong_update();
    	render_pong_buffer(game_state);
      }
      else if(display_mode==pendulumMode){
        display_buffer(PONG_BUFFER);  // reuse the Pong buffer
        clear_buffer(PONG_BUFFER);
        render_pendulum_buffer();
      }

    }
    if(verbose_mode){
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
    // tweak error_term used to sync pendulum with seconds:
      error_term = (cycle_count % 31250);
      updateTimeDisplay();
      time_has_passed = 0;     
    } 
    
    display_mode = QuadDec_1_GetCounter() % 4;
    poll_button();
  }
   

#define SCOPE_DELAY_SHORT 96
}

/* [] END OF FILE */
