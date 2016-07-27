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
#include "menus.h"
#include "prefs.h"
#include "max509.h"
#include "fourletter.h"
#include "gps.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Real time clock variables:
volatile int second_has_elapsed = 0;
//int led_state = 0;  // we blink this once/second


typedef enum{flwMode, textMode,analogMode, pongMode,pendulumMode,gpsDebugMode,menuMode} clock_type;
clock_type display_mode=pendulumMode;

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
    strobe_LDAC();
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

// Show a four letter word:
void compile_flw(){
    char *rw;
    static int lastUpdate=0;
    
    if(cycle_count-lastUpdate > 31250){  // one second update interval..
        rw = random_word();
        compileString(rw,255,88,ANALOG_BUFFER,5,OVERWRITE);
        lastUpdate = cycle_count;
        
    }
}

// test case data:
seg_or_flag test_segs[] = {
  {128,128,255,255,cir,0xff},
  {128,128,96,96,neg,0x99},
  {128,128,96,0,pos,0x99},
  {128,128,0,96,pos,0x99},
  {255,255,0,0,cir,0xff},
};

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
//  float x = 128.0 + (SEC_HAND_LENGTH-4)*sin(2*M_PI*(cycle_count-error_term)/31250.0);
//  float y = 128.0 + (SEC_HAND_LENGTH-4)*cos(2*M_PI*(cycle_count-error_term)/31250.0);
//  circle(x,y,16,ANALOG_BUFFER);
}

/* ************* Pong Game ************* */
#define PADDLE_HEIGHT 24
#define PADDLE_WIDTH   8
#define PONG_TOP 240
#define PONG_BOTTOM 0
#define PONG_LEFT PADDLE_WIDTH
#define PONG_RIGHT 255-PADDLE_WIDTH
#define PADDLE_MIN PONG_BOTTOM+(PADDLE_HEIGHT/2)+1
#define PADDLE_MAX PONG_TOP-(PADDLE_HEIGHT/2)-1

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
  if(game_state.puck_position[0] < PONG_LEFT) return(1);
  if(game_state.puck_position[0]> PONG_RIGHT) return(2);
  if(game_state.puck_position[1] < PONG_BOTTOM) return(3);
  if(game_state.puck_position[1 ]> PONG_TOP) return(4);

  return(0);
       
}
int puck_visible(){
  if(game_state.puck_position[0] > 0 || game_state.puck_position[0]<255)
    return 1;
  else 
    return 0;
}
void constrain(int *x, int xmin, int xmax){
    if(*x>xmax) *x = xmax;
    if(*x < xmin) *x = xmin;
}
void update_paddles(){
  int player;
  RTC_1_TIME_DATE *the_time;
  the_time = RTC_1_ReadTime();
  int should_miss[2] = {0,0}; //set to 1 if we want that player to miss
  
  if(puck_visible()){
    if((the_time->Min ==59 && the_time->Sec>57) || game_state.puck_velocity[0]<0) should_miss[1]=1;
    else if((the_time->Sec > 57) || game_state.puck_velocity[0]>0) should_miss[0]=1;

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
  if(game_state.puck_velocity[0]<0 && abs(game_state.puck_position[0]-PADDLE_WIDTH) <= abs(game_state.puck_velocity[0]))
    which_paddle = 0;
  else if(game_state.puck_velocity[0]>0 && abs(game_state.puck_position[0]-(PONG_RIGHT-PADDLE_WIDTH)) <= abs(game_state.puck_velocity[0]))
    which_paddle=1;
  else return 0;
  
  result = game_state.puck_position[1]-game_state.paddle_position[which_paddle];
        
  if(abs(result) > PADDLE_HEIGHT/2) result=0;  // we missed

  return (result / 8) + (rand() % 3)-1;  
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
  compileString(time_str,32,200,PONG_BUFFER,2,APPEND);
    
  sprintf(time_str,"%02i",the_minute);
  compileString(time_str,174,200,PONG_BUFFER,2,APPEND);
    
}

/*  Pendulum Clock *** */
void render_pendulum_buffer(){
  RTC_1_TIME_DATE *the_time;
  char sec_str[32],hr_min_string[32];
  float x,y,i;
  the_time = RTC_1_ReadTime();

  // render the time in seconds
  sprintf(sec_str,"%02i",the_time->Sec);
  compileString(sec_str,255,32,PONG_BUFFER,2,OVERWRITE);

  // render the hour and minute:  
  sprintf(hr_min_string,"%02i:%02i",the_time->Hour,the_time->Min);
  compileString(hr_min_string,255,130,PONG_BUFFER,3,APPEND);

  // render the pendulum shaft:  
  x = 128.0+200*sin(sin(2*M_PI*(cycle_count-error_term)/31250.0)/2.5);
  y = 250.0 - 200*cos(sin(2*M_PI*(cycle_count-error_term)/31250.0)/2.5);
  line(128,250,x,y,PONG_BUFFER);

  //render the pendulum bob:
  for(i=32;i>0;i-=8) circle(x,y,i,PONG_BUFFER);

  //render the point from which the pendulum swings:
  circle(128,250,8,PONG_BUFFER);

}


void display_buffer(uint8 which_buffer){
  //long start_count = cycle_count;
  seg_or_flag *seg_ptr = seg_buffer[which_buffer];
  while(seg_ptr->seg_data.x_offset != 0xff){

    if(current_state==blank_unprimed){  
      uint8 int_status = CyEnterCriticalSection();
            
      set_DACfor_seg(seg_ptr,0,0);
      switch(seg_ptr->seg_data.arc_type){
        case cir:
          Phase_Register_Write(0x1);
          break;
        
        case pos:
          Phase_Register_Write(0x0);
          break;
        
        case neg:
          Phase_Register_Write(0x2);
          break;
    }
              
      times_to_loop = (seg_ptr->seg_data.x_size>seg_ptr->seg_data.y_size) ? \
	  seg_ptr->seg_data.x_size/6 : seg_ptr->seg_data.y_size/6;
      if(times_to_loop==0) times_to_loop = 1;
      if(seg_ptr->seg_data.arc_type == cir) times_to_loop *= 2;  // circles don't double up like lines
    
    // test:
     // times_to_loop=1;

     
      // performance measurement:
      if(which_buffer != DEBUG_BUFFER) loops_per_frame+=times_to_loop+1;
      current_mask = seg_ptr->seg_data.mask;
      // EXPERIMENT:
//      if(current_mask==0x99){
//        current_mask = 0x81;
//        times_to_loop *= 2;
//    }
//      if(seg_ptr->seg_data.arc_type != cir) current_mask ^= 0xff;
      ShiftReg_1_WriteData(current_mask);  // "prime" the shift register

      current_state = blank_primed;
     // strobe_LDAC();  experiment:  do this in the interrupt routine
      seg_ptr++;
          
      CyExitCriticalSection(int_status);
    }
    else{
         // check serial port for gps characters:
        while(UART_1_GetRxBufferSize()>0){
            char c = UART_1_GetByte() & 0x00ff;
            consume_char(c);
        }
   
    }
  }
}

void initTime(){
  RTC_1_TIME_DATE *the_time = RTC_1_ReadTime();
  RTC_1_DisableInt();
    
  the_time->Month = 3;
  the_time->DayOfMonth = 1;
  the_time->DayOfWeek=0;
  the_time->Year = 2016;
  the_time->Hour = 05;
  the_time->Min = 59;
  the_time->Sec = 50;

offset_time(the_time,-7);

  RTC_1_WriteTime(the_time);
  RTC_1_WriteIntervalMask(RTC_1_INTERVAL_SEC_MASK);
  RTC_1_EnableInt();
  RTC_1_Start(); //done in gps_init at the moment...  
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
    
    
    
  sprintf(time_string,"%i:%02i:%02i",hours,minutes,seconds);
  compileString(time_string,255,46,ANALOG_BUFFER,3,OVERWRITE);

  //    sprintf(time_string,"%i:%02i",hours,minutes);
  //    compileString(time_string,0,0,4);

  sprintf(date_string,"%s %i, %i",month_names[month-1],day_of_month,year);
 
  compileString(date_string,255,142,ANALOG_BUFFER,1,APPEND);
     
  char dw[12];
  sprintf(dw,"%s",day_names[day_of_week-1]);
  compileString(dw,255,202,ANALOG_BUFFER,2,APPEND);


}
void waitForClick(){
    while(!button_clicked);
    button_clicked=0;
}
void hw_test(){
  SPIM_1_WriteTxData(DAC_Reg_A | DAC_Pre_Load |0x80);
  CyDelay(1);
  SPIM_1_WriteTxData(DAC_Reg_B | DAC_Pre_Load |0x80);

//  AMux_1_Select(shape_to_mux[cir]);
        
  SPIM_1_WriteTxData(DAC_Reg_C | DAC_Load_Now | 0x80);
  CyDelay(1);
  SPIM_1_WriteTxData(DAC_Reg_D | DAC_Load_Now | 0x80);

  CyDelay(1);
  strobe_LDAC();
  waitForClick();

  int i=0,j=0;
  int levels[] = {0,128,254};
  for(i=0;i<3;i++){
      SPIM_1_WriteTxData(DAC_Reg_A | DAC_Load_Now | levels[i]);
    CyDelay(1);
      SPIM_1_WriteTxData(DAC_Reg_B | DAC_Load_Now | levels[i]);
    CyDelay(1);
    strobe_LDAC();
    waitForClick();
  }
    SPIM_1_WriteTxData(DAC_Reg_A | DAC_Load_Now | 0x10);
    CyDelay(1);
      SPIM_1_WriteTxData(DAC_Reg_B | DAC_Load_Now | 0x10);
    CyDelay(1);
    strobe_LDAC();
  for(i=0;i<3;i++){
    for(j=0;j<3;j++){
      SPIM_1_WriteTxData(DAC_Reg_C | DAC_Load_Now | levels[i]);
      CyDelay(1);
      SPIM_1_WriteTxData(DAC_Reg_D | DAC_Load_Now | levels[j]);
      CyDelay(1);
      strobe_LDAC();
      waitForClick();
        
    }
  }
  waitForClick();
  clear_buffer(0);
  vector_font c={{192,192,128,128,pos,0xff}, {255,0,0,0,0,0,}};
  //compileSegments(c,0,0);
  circle(64,64,64,0);
  
  while(!button_clicked)display_buffer(0);
  button_clicked=0;
}

int main() 
{
    int last_switch = 0;
  /* Start up the SPI interface: */
  SPIM_1_Start();
    
  /* Start VDACs */
  VDAC8_1_Start();
  VDAC8_2_Start();
    
    
 
  /* Initialize the shift register: */
  ShiftReg_1_Start();

  // Start the quadrature decoder(aka "the knob"):
  QuadDec_1_Start();

  // Initialize button interrupt (which is a routine that polls the button at 60Hz):
  button_isr_Start();

  // initialize the pseudo 1pps interrupt from the gps:
  //one_pps_int_Start();
    
  /* Initialize Wave Interrupt, which manages the circles: */
  isr_1_StartEx(wave_started);
  CyGlobalIntEnable;

// start the UART for gps communications:
 init_gps();

  //start the real-time clock component (since the system is a clock, after all)
  // When GPS is enabled, we don't call RTC_1_Start, since GPS supplies the 1 pps
  //initTime();
   

  /* initialize sysfont: */
  init_font();

 /* initialize the four letter word randomizer: */
  init_flws();

// initialize the EEPROM for saving prefs:
 init_prefs();
    
  CyDelay(100);
  //hw_test();
 // dispatch_menu(0,2);
 // dispatch_menu(0,3);

  uint8 toggle_var=0;
  
  for(;;){
    if(second_has_elapsed){
        LED_Reg_Write(toggle_var);
        toggle_var=1-toggle_var;
    }
    if(second_has_elapsed && (display_mode != menuMode)){
      // tweak error_term used to sync pendulum with seconds:
      error_term = (cycle_count % 31250);
    } 
    second_has_elapsed = 0;     
    RTC_1_TIME_DATE *now;
//    int phase = SixtyHz_Read();
//    while(SixtyHz_Read() == phase);   // wait for a 60Hz edge..
    
    switch (display_mode){

    case gpsDebugMode:
      
      compile_substring(sentence,32,255,64,ANALOG_BUFFER,1,OVERWRITE);
      compile_substring(&sentence[32],32,255,32,ANALOG_BUFFER,1,APPEND);
      display_buffer(ANALOG_BUFFER);
      break;
    
     case flwMode:
      compile_flw();
      display_buffer(ANALOG_BUFFER);
      break;
    
    case textMode:
      updateTimeDisplay();
      display_buffer(0);
      break;
    
    case analogMode:
      now = RTC_1_ReadTime();
      updateAnalogClock(now->Hour,now->Min,now->Sec);
      display_buffer(ANALOG_BUFFER);
      break;
    
    case pongMode:
      display_buffer(PONG_BUFFER);
      pong_update();
      render_pong_buffer(game_state);
      break;

    case pendulumMode:
      display_buffer(PONG_BUFFER);  // reuse the Pong buffer
      clear_buffer(PONG_BUFFER);
      render_pendulum_buffer();
      break;

    case menuMode:
      display_menu(main_menu);
      display_buffer(MENU_BUFFER);
      break;

    }

  //update the interim screen-saver:
  RTC_1_TIME_DATE *t=RTC_1_ReadTime();
  ss_x_offset = (t->Min) % 5;
  ss_y_offset =(t->Min+2) % 5;
 
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
    
    int interval = global_prefs.prefs_data.switch_interval;
    
    if(display_mode != menuMode && interval==0) display_mode = QuadDec_1_GetCounter() % 6;
    else main_menu.highlighted_item_index = QuadDec_1_GetCounter() % (main_menu.n_items);
    if(display_mode == menuMode) main_menu.highlighted_item_index = QuadDec_1_GetCounter() % (main_menu.n_items);
    
    if(button_clicked){
        button_clicked=0;  // consume the click
        if(display_mode==menuMode){
            dispatch_menu(main_menu.menu_number,main_menu.highlighted_item_index);
            display_mode = textMode;
        }
        else display_mode = menuMode;
    }
    else{
     if(display_mode != menuMode && interval!=0 && cycle_count-last_switch > interval*31250){
       display_mode = (cycle_count / (interval*31250)) % 5;   // switch every 10 seconds
        last_switch=cycle_count;  
    }
    }
  }
}

/* [] END OF FILE */
	
