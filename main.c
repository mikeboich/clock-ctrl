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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Real time clock variables:
volatile int pps_available=0;


typedef enum{textMode,flwMode,analogMode1, secondsOnly,pongMode,pendulumMode,trump_elapsed_mode, \
    trumpMode,xmasMode,wordClockMode,analogMode0,analogMode2,gpsDebugMode,julianDate,menuMode} clock_type;
int nmodes = 14;
int n_auto_modes=11;
clock_type display_mode=pendulumMode;
clock_type saved_mode;

int verbose_mode = 0;

typedef enum{blank_unprimed,blank_primed,drawing,last_period,hw_testing}  draw_state;  // States of the draw loop/interrupt code

uint8 current_mask=0;
volatile draw_state current_state = blank_unprimed;
volatile int current_phase=0;  // phase of sin lookup machinery

volatile int times_to_loop = 0;
volatile uint64_t cycle_count=0;  // poor man's timer
int frame_toggle = 0;   // performance measurement
volatile uint64_t phase_error=0;            // difference between (cycle_count % 31250) and 1 pps edge
volatile uint64_t minute_error=0;            // difference between (cycle_count % 60*31250) and 1 minute boundary

int last_refresh=0,loops_per_frame=0;   // for performance measurement

/* Define the start-of-segment interrupt routine 
   This routine loads the next 8 bit segment mask from the display list,
   and counts the number of drawing passes/segment.
*/

CY_ISR_PROTO(wave_started);

void wave_started(){
  isr_1_ClearPending();       // clear the interrupt
  cycle_count++;
  switch(current_state){

  case hw_testing:
    ShiftReg_1_WriteData(0xff);
    break;

  case blank_unprimed:
    ShiftReg_1_WriteData(0x0);
    break;

  case last_period:
    ShiftReg_1_WriteData(0x0);  // next period is blanked
    current_state = blank_unprimed;
    break;

  case blank_primed:
    current_state = drawing;
    Phase_Register_Write(current_phase);
    strobe_LDAC();
    break;
    
  case drawing:
    times_to_loop -= 1;
    if(times_to_loop==0){
      ShiftReg_1_WriteData(0x0);  // blank on the next cycle
      current_state = last_period;
    }
    else {
      ShiftReg_1_WriteData(current_mask);
    }
    break;
  }
}

void renderGPSDebug(RTC_1_TIME_DATE *now){
  RTC_1_TIME_DATE utc_time = *now;
  char pe[64],me[64];

  offset_time(&utc_time,-global_prefs.prefs_data.utc_offset);
  char time_string[32];
  char day_of_week_string[12];
  char date_string[15];

  int seconds = utc_time.Sec;
  int minutes = utc_time.Min;
  int hours = utc_time.Hour;
    
        
  sprintf(time_string,"%i:%02i:%02i UTC",hours,minutes,seconds);
  compileString(time_string,255,128-32,MAIN_BUFFER,1,OVERWRITE); 

  int month = utc_time.Month;  
  int day = utc_time.DayOfMonth;
  int year = utc_time.Year;

  sprintf(date_string,"%02i/%02i/%04i",month,day,year);
  compileString(date_string,255,128+32,MAIN_BUFFER,1,APPEND); 

  
//  sprintf(pe,"phase error: %Lu",phase_error);
//  compileString(pe,255,128-64,MAIN_BUFFER,1,APPEND);
//  sprintf(pe,"minute err: %Lu",minute_error);
//  compileString(pe,255,128-32,MAIN_BUFFER,1,APPEND);


}

// Show a four letter word:
void render_flw(){
  char *rw;
  static int lastUpdate=0;
    
  if(cycle_count-lastUpdate > 31250){  // one second update interval..
    rw = random_word();
    //rw = next_word();  // uncomment this line to have sequential, rather than random words
    compileString(rw,255,88,MAIN_BUFFER,5,OVERWRITE);
    lastUpdate = cycle_count;
        
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

  if(minute_error){
    float smooth_angle = 2*M_PI* (((cycle_count - minute_error)/(60*31250.0)));
    second_angle = smooth_angle;
}


  // not doing the 2-d hands yet, just lines
  line(128,128,128 + sin(hour_angle)*HR_HAND_LENGTH,128 + cos(hour_angle) * HR_HAND_LENGTH,MAIN_BUFFER);  // draw the hour hand
  line(128,128,128 + sin(minute_angle)*MIN_HAND_LENGTH ,128 + cos(minute_angle) * MIN_HAND_LENGTH,MAIN_BUFFER);
  if(display_mode < analogMode2){
    line(128,128,128 + sin(second_angle)*SEC_HAND_LENGTH,128 + cos(second_angle) * SEC_HAND_LENGTH,MAIN_BUFFER);
  }

}

void renderAnalogClockBuffer(RTC_1_TIME_DATE *now){
  int i;
  float angle=0.0;
  static char *nums[12] = {"12","1","2","3","4","5","6","7","8","9","10","11"};
  seg_or_flag face[] = {{128,128,255,255,cir,0xff},
			{128,128,8,8,cir,0xff},
			{.flag=0xff}};
  compileSegments(face,MAIN_BUFFER,OVERWRITE);
  compileString("12",112,216,MAIN_BUFFER,1,APPEND);
  compileString("6",120,20,MAIN_BUFFER,1,APPEND);
  compileString("3",220,120,MAIN_BUFFER,1,APPEND);
  compileString("9",20,120,MAIN_BUFFER,1,APPEND);
    
  drawClockHands(now->Hour,now->Min,now->Sec);

  if(display_mode == analogMode0){
    // experimental one revoultion/second widget:
    float x = 128.0 + (SEC_HAND_LENGTH-4)*sin(2*M_PI*(cycle_count-phase_error)/31250.0);
    float y = 128.0 + (SEC_HAND_LENGTH-4)*cos(2*M_PI*(cycle_count-phase_error)/31250.0);
    circle(x,y,16,MAIN_BUFFER);
  }
}

asm (".global _scanf_float");       // forces floating point formatting code to load
// for displaying things like "400 days of Trump to go!" or "332 shopping days till Christmas!":
// the 
void countdown_to_event(RTC_1_TIME_DATE *now, time_t  event_time,char *caption0, char *caption1){
    time_t current_time;
    struct tm tm_now;
    double seconds_remaining;
    double days_remaining;
    char seconds_string[64] = "";
    
    tm_now.tm_year = now->Year-1900;
    tm_now.tm_mon = now->Month-1;       // months are 0..11 rather than 1..12!
    tm_now.tm_mday = now->DayOfMonth;
    tm_now.tm_hour = now->Hour;
    tm_now.tm_min = now->Min;
    tm_now.tm_sec = now->Sec;
    tm_now.tm_isdst=0;
    current_time = mktime(&tm_now);
    
    
    seconds_remaining = difftime(event_time,current_time);
    days_remaining = fabs(seconds_remaining/86400.0);  
    
    
    sprintf(seconds_string,"%.5f",days_remaining);
    
    compileString(seconds_string,255,140,MAIN_BUFFER,2,OVERWRITE);
    compileString(caption0,255,90,MAIN_BUFFER,1,APPEND);
    compileString(caption1,255,40,MAIN_BUFFER,1,APPEND);
    
    
}
void render_trump_elapsed_buffer(RTC_1_TIME_DATE *now){
    time_t end_time,start_time;
    struct tm end_of_trump,start_of_trump;
    end_of_trump.tm_year = 2021-1900;
    start_of_trump.tm_year = 2017-1900;
    start_of_trump.tm_mon = end_of_trump.tm_mon = 1-1;   // months are 0..11 rather than 1..12!
    start_of_trump.tm_mday = end_of_trump.tm_mday = 20;
    start_of_trump.tm_hour = end_of_trump.tm_hour = 17 + global_prefs.prefs_data.utc_offset;  // local time corresponding to 1700UTC
    start_of_trump.tm_min = end_of_trump.tm_min = 0;
    start_of_trump.tm_sec = end_of_trump.tm_sec = 0;
    
    end_time = mktime(&end_of_trump);
    start_time = mktime(&start_of_trump);
    
    
    countdown_to_event(now,start_time,"Days of Trump","elapsed");
}

void render_trump_buffer(RTC_1_TIME_DATE *now){
    time_t end_time,start_time;
    struct tm end_of_trump,start_of_trump;
    start_of_trump.tm_year = end_of_trump.tm_year = 2021-1900;
    start_of_trump.tm_mon = end_of_trump.tm_mon = 1-1;   // months are 0..11 rather than 1..12!
    start_of_trump.tm_mday = end_of_trump.tm_mday = 20;
    start_of_trump.tm_hour = end_of_trump.tm_hour = 17 + global_prefs.prefs_data.utc_offset;  // local time corresponding to 1700UTC
    start_of_trump.tm_min = end_of_trump.tm_min = 0;
    start_of_trump.tm_sec = end_of_trump.tm_sec = 0;
    
    end_time = mktime(&end_of_trump);
    start_time = mktime(&start_of_trump);
    
    
    countdown_to_event(now,end_time,"Days of Trump","remaining");
}
void render_xmas_buffer(RTC_1_TIME_DATE *now){
    time_t end_time;
    struct tm xmas_time;
    
    xmas_time.tm_year = now->Year-1900;
    xmas_time.tm_mon = 12-1;   // months are 0..11 rather than 1..12!
    xmas_time.tm_mday = 25;
    xmas_time.tm_hour = 0;  //  midnight local time
    xmas_time.tm_min = 0;
    xmas_time.tm_sec = 0;
    if(now->Month == 12 && now->DayOfMonth>25){
        xmas_time.tm_year += 1;
    }
    end_time = mktime(&xmas_time);
    
    countdown_to_event(now,end_time,"Shopping Days","until Christmas!");
}

double mod_julian_date(RTC_1_TIME_DATE *now){
    RTC_1_TIME_DATE local_now = *now;
    offset_time(&local_now,-global_prefs.prefs_data.utc_offset);  // work un utc
    int y,m,d;
    y = local_now.Year;
    m = local_now.Month;
    d = local_now.DayOfMonth;
    if(m < 3){
        y = y-1;
        m = m+12;
        }
    int a = trunc(y/100);
    int b = 2 - a + trunc(a/4);
    double julian_day = trunc(365.25 * (y + 4716)) +  trunc(30.6001 * (m + 1)) + d + b - 1524.5;
    
    int seconds_past_midnight = 3600*local_now.Hour + 60*local_now.Min + local_now.Sec;
    double fraction = seconds_past_midnight / 86400.0;
    return julian_day + fraction - 2400000.5;
}
// renders the modifed Julian date, which Julian date - 2400000.5:
void render_julian_date(RTC_1_TIME_DATE *now){
    double jd = mod_julian_date(now);
    char jd_str[32];
    
    sprintf(jd_str,"Modified Julian Date:");
    compileString(jd_str,255,128+32,MAIN_BUFFER,1,OVERWRITE);
    sprintf(jd_str,"%.5f",jd);
    compileString(jd_str,255,128-32,MAIN_BUFFER,1,APPEND);
}

void render_word_clock(RTC_1_TIME_DATE *now){
    char *strs[] = {"o'clock","b"};
    char *hour_strings[] = {"twelve","one","two","three","four","five","six","seven","eight","nine","ten","eleven"};
    char *minute_strings[] = {"not-used","five","ten","a quarter","twenty","twenty-five"};
    char past_or_until[8];
    int exact=0;
    
    char time_string[3][64];
    
    if(now->Min > 57 || now->Min<3){
        if(now->Min==0)
            sprintf(time_string[0],"It's exactly");
        else
            sprintf(time_string[0],"It's about");
        compileString(time_string[0],255,160,MAIN_BUFFER,2,OVERWRITE);
        int the_hour = now->Min > 56 ? now->Hour+1 : now->Hour;
        sprintf(time_string[0],"%s ",hour_strings[the_hour % 12]);
        compileString(time_string[0],255,108,MAIN_BUFFER,2,APPEND);
        sprintf(time_string[0],"O'clock");
        compileString(time_string[0],255,50,MAIN_BUFFER,2,APPEND);
        return;
    }
    
    if(now->Min >=3 && now->Min <=57){
        if(now->Min > 27 && now->Min < 33){
            if(now->Min==30)
                compileString("It's exactly",255,150,MAIN_BUFFER,2,OVERWRITE);
            else
                compileString("It's about",255,150,MAIN_BUFFER,2,OVERWRITE);
                
            compileString("half past",255,100,MAIN_BUFFER,2,APPEND);
            sprintf(time_string[0],"%s",hour_strings[ now->Hour % 12]);
            compileString(time_string[0],255,50,MAIN_BUFFER,2,APPEND);
            return;
        }
        else{
            // round to nearest 5 minutes:
            int approx_minute = 5*(now->Min / 5);
            int past_until_index;
            if((now->Min - approx_minute) > 2){
                approx_minute += 5;
            }
            exact = (approx_minute ==  now->Min);
            
            if(exact)
                compileString("It's exactly",255,200,MAIN_BUFFER,2,OVERWRITE);
            else
                compileString("It's about",255,200,MAIN_BUFFER,2,OVERWRITE);
            if(now->Min <= 27){
                past_until_index = approx_minute / 5;
                sprintf(time_string[0],"%s",minute_strings[past_until_index]);
                compileString(time_string[0],255,150,MAIN_BUFFER,2,APPEND);
                sprintf(time_string[0],"past");
                compileString(time_string[0],255,100,MAIN_BUFFER,2,APPEND);
                sprintf(time_string[0],"%s",hour_strings[now->Hour % 12]);
                compileString(time_string[0],255,50,MAIN_BUFFER,2,APPEND);
          
            }
            if(now->Min >= 33){
                approx_minute = 60-approx_minute;
                past_until_index = approx_minute / 5;
                sprintf(time_string[0],"%s",minute_strings[(approx_minute/5)]);
                compileString(time_string[0],255,150,MAIN_BUFFER,2,APPEND);                
                sprintf(time_string[0],"'till");
                compileString(time_string[0],255,100,MAIN_BUFFER,2,APPEND);                
                sprintf(time_string[0],"%s",hour_strings[((now->Hour+1)) % 12]);
                compileString(time_string[0],255,50,MAIN_BUFFER,2,APPEND);                
            }
/*           if(now->Min >= 28 && now->Min<=32){
                compileString("It's about",255,200,MAIN_BUFFER,2,OVERWRITE);
                past_until_index = approx_minute / 5;
                sprintf(time_string[0],"half");
                compileString(time_string[0],255,150,MAIN_BUFFER,2,APPEND);
                sprintf(time_string[0],"past");
                compileString(time_string[0],255,100,MAIN_BUFFER,2,APPEND);
                sprintf(time_string[0],"%s",hour_strings[now->Hour % 12]);
                compileString(time_string[0],255,50,MAIN_BUFFER,2,APPEND);
            }
*/
        }
    }
    
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
  .puck_velocity = {6,3},
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
    should_miss[1] = ((the_time->Min ==59 && the_time->Sec>58) || game_state.puck_velocity[0]<0) ? 1 :0;
    should_miss[0] = ((the_time->Sec > 58) || game_state.puck_velocity[0]>0) ? 1 : 0;

    for(player=0;player<2;player++){
      if(!should_miss[player]){
    	if(game_state.paddle_position[player] > game_state.puck_position[1] && game_state.paddle_position[player] > PADDLE_MIN)
    	  game_state.paddle_position[player] -= 4;
             
    	if (game_state.paddle_position[player] < game_state.puck_position[1] && \
    	    game_state.paddle_position[player] < PADDLE_MAX) game_state.paddle_position[player] += 4;

          }
    else{
    	if(game_state.paddle_position[player] > game_state.puck_position[1]) 
          if(game_state.paddle_position[player] < PADDLE_MIN){
    	    game_state.paddle_position[player] += 4;
        }
        
             
    	if (game_state.paddle_position[player] < game_state.puck_position[1] && \
    	    game_state.paddle_position[player] > PADDLE_MAX) game_state.paddle_position[player] -= 4;
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

void render_pong_buffer(pong_state the_state,RTC_1_TIME_DATE *the_time){
  int x,y;
  
  clear_buffer(MAIN_BUFFER);
  // draw the left paddle
  for(y=the_state.paddle_position[0]-(PADDLE_HEIGHT/2);y<the_state.paddle_position[0]+(PADDLE_HEIGHT/2)+1;y++) \
    line(0,y,PADDLE_WIDTH,y,MAIN_BUFFER);
  // draw the right paddle:
  for(y=the_state.paddle_position[1]-(PADDLE_HEIGHT/2);y<the_state.paddle_position[1]+(PADDLE_HEIGHT/2)+1;y++) \
    line(254-PADDLE_WIDTH,y,254,y,MAIN_BUFFER);
    
  // draw puck:
  x = the_state.puck_position[0];
  if(puck_visible()){
    for(y=the_state.puck_position[1]-2;y<the_state.puck_position[1]+3;y++)
      line(x-2,y,x+2,y,MAIN_BUFFER);
  }
    
  // draw the centerline:
  x=128;
  for(y=240;y>0;y-=32){
    line(128,y,128,y-16,MAIN_BUFFER);   
  }
    
  // draw the hours and minutes as two scores:
  char time_str[32];
  int the_hour = the_time->Hour;
  int the_minute = the_time->Min;
  sprintf(time_str,"%02i",the_hour);
  compileString(time_str,36,200,MAIN_BUFFER,2,APPEND);
    
  sprintf(time_str,"%02i",the_minute);
  compileString(time_str,160,200,MAIN_BUFFER,2,APPEND);
    
}

/*  Pendulum Clock *** */
void render_pendulum_buffer(RTC_1_TIME_DATE *the_time){
  char sec_str[32],hr_min_string[32];
  float x,y,i;

  // render the time in seconds
  sprintf(sec_str,"%02i",the_time->Sec);
  compileString(sec_str,255,32,MAIN_BUFFER,2,OVERWRITE);

  // render the hour and minute:  
  sprintf(hr_min_string,"%02i:%02i",the_time->Hour,the_time->Min);
  compileString(hr_min_string,255,115,MAIN_BUFFER,3,APPEND);

  // render the pendulum shaft:  
  x = 128.0+200*sin(sin(2*M_PI*(cycle_count-phase_error)/31250.0)/2.5);
  y = 250.0 - 200*cos(sin(2*M_PI*(cycle_count-phase_error)/31250.0)/2.5);
  line(128,250,x,y,MAIN_BUFFER);

  //render the pendulum bob:
  for(i=32;i>0;i-=8) circle(x,y,i,MAIN_BUFFER);

  //render the point from which the pendulum swings:
  circle(128,245,8,MAIN_BUFFER);

}

void render_text_clock(RTC_1_TIME_DATE *the_time){
  char time_string[32];
  char day_of_week_string[12];
  char date_string[15];

  int seconds = the_time->Sec;
  int minutes = the_time->Min;
  int hours = the_time->Hour;
    
  int day_of_week = the_time->DayOfWeek;
  int month = the_time->Month;
  int day_of_month = the_time->DayOfMonth;
  int year = the_time->Year;
        
  sprintf(time_string,"%i:%02i:%02i",hours,minutes,seconds);
  compileString(time_string,255,46,MAIN_BUFFER,3,OVERWRITE);

  sprintf(date_string,"%s %i, %i",month_names[month-1],day_of_month,year);
 
  compileString(date_string,255,142,MAIN_BUFFER,1,APPEND);
     
  char dw[12];
  sprintf(dw,"%s",day_names[day_of_week-1]);
  compileString(dw,255,202,MAIN_BUFFER,2,APPEND);
}

void renderSeconds(RTC_1_TIME_DATE *the_time){
    char sec_str[4];
    char hour_min_str[8];
    char day_of_week_str[16];
    
    sprintf(hour_min_str,"%02i:%02i",the_time->Hour ,the_time->Min);
    sprintf(sec_str,"%02i",the_time->Sec);
    sprintf(day_of_week_str,"%s",day_names[the_time->DayOfWeek-1]);
    compileString(sec_str,255,10,MAIN_BUFFER,2,OVERWRITE);
    compileString(hour_min_str,255,85,MAIN_BUFFER,4,APPEND);
    compileString(day_of_week_str,255,205,MAIN_BUFFER,2,APPEND);
}
uint8_t cordicSqrt(uint16_t value){
    uint8_t delta,loop,result;
    
    for(delta=0b10000000,result=0,loop=1;loop<=8; loop++){
        result |= delta;
        if(((uint16_t)result * ((uint16_t)result)) > value)
          result &= ~delta;
        delta >>= 1;
    }
    return result;
}
void display_buffer(uint8 which_buffer){
#define PI 3
    
  seg_or_flag *seg_ptr = seg_buffer[which_buffer];
  FrameDrawReg_Write(frame_toggle);
  frame_toggle = 1 - frame_toggle;
  sync_to_60Hz();
  while(seg_ptr->seg_data.x_offset != 0xff){

    if(current_state==blank_unprimed){
      uint8 int_status = CyEnterCriticalSection();
            
      set_DACfor_seg(seg_ptr,ss_x_offset,ss_y_offset);
      switch(seg_ptr->seg_data.arc_type){
      case cir:
        current_phase=0x1;   // phase register can't be written here, as drawing may still be active, so set current_phase instead
	break;
        
      case pos:
	  current_phase = 0x0;
	break;
        
      case neg:
        current_phase = 0x2;
	break;
      }
#undef SGI_TEACH   
#ifdef SGI_TEACH    
      // trying SGITeach brightness algorithm, vs my stupid very simple one:
#define SQR(a) a*a
#define LINE_DIM_OCTANT 0x81
#define CIRCLE_DIM_OCTANT 0x55
#define CIRCLE_VERY_DIM_OCTANT 0x05
#define CIRCLE_VERY_VERY_DIM_OCTANT  0x1


  if(seg_ptr->seg_data.arc_type == cir){
	if(seg_ptr->seg_data.x_size == seg_ptr->seg_data.y_size)
	  times_to_loop = PI*seg_ptr->seg_data.x_size;  // circle case
	else{
	  uint16 a = seg_ptr->seg_data.x_size/2;
	  uint16 b = seg_ptr->seg_data.y_size/2;
	  times_to_loop = PI*(3*(a+b) - cordicSqrt((3*a+b)*(a+3*b))); // ellipse case         
	}
    
	if(times_to_loop < 10){
	seg_ptr->seg_data.mask = CIRCLE_VERY_DIM_OCTANT;
	}
	if(times_to_loop < 5){
	seg_ptr->seg_data.mask = CIRCLE_VERY_VERY_DIM_OCTANT;
	}
      }
      else{  // line of some sort..
	if(seg_ptr->seg_data.y_size==0)
	  times_to_loop = seg_ptr->seg_data.x_size;
	else
	  if(seg_ptr->seg_data.x_size==0)
	    times_to_loop = seg_ptr->seg_data.y_size;
	  else times_to_loop = sqrt(SQR((uint16_t)seg_ptr->seg_data.x_size) + SQR((uint16_t)seg_ptr->seg_data.y_size));
	times_to_loop = PI * times_to_loop/2;  // lines are drawn twice
    if(times_to_loop < 20)
      seg_ptr->seg_data.mask = LINE_DIM_OCTANT;
  }

      if(times_to_loop < 20)
	times_to_loop = 2;
      else
	times_to_loop /=10;

#else      

      // old brightness routine:
      times_to_loop = (seg_ptr->seg_data.x_size>seg_ptr->seg_data.y_size) ? \
      seg_ptr->seg_data.x_size/6 : seg_ptr->seg_data.y_size/6;
      if(times_to_loop==0) times_to_loop = 1;
      if(seg_ptr->seg_data.arc_type == cir) times_to_loop *= 2;  // circles don't double up like lines
    
#endif    
      // performance measurement:
      if(which_buffer != DEBUG_BUFFER) loops_per_frame+=times_to_loop+1;
    
      current_mask = seg_ptr->seg_data.mask;

      ShiftReg_1_WriteData(current_mask);  // "prime" the shift register

      current_state = blank_primed;
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
  RTC_1_WriteIntervalMask(RTC_1_INTERVAL_SEC_MASK | RTC_1_INTERVAL_MIN_MASK);
  RTC_1_EnableInt();
  RTC_1_Start(); //done in gps_init at the moment...  
}


void waitForClick(){
  while(!button_clicked);
  button_clicked=0;
}

#define LINELEN 128
void hw_test(){
  seg_or_flag test_pattern[] = {
    {128,128,128,128,cir,0xff},
    {255,255,0,0,cir,0x00},
  }; 

  set_DACfor_seg(test_pattern,0,0);
  strobe_LDAC();
  ShiftReg_1_WriteData(0xff);
  Phase_Register_Write(0x2);
  current_state = hw_testing;
    while(!button_clicked){
    display_buffer(MAIN_BUFFER);
  }
  button_clicked = 0;  // consume the button_click
  current_state = blank_unprimed;

  clear_buffer(MAIN_BUFFER);
  compileSegments(test_pattern,MAIN_BUFFER,APPEND);
    
    
  while(!button_clicked){
    display_buffer(MAIN_BUFFER);
  }
  button_clicked = 0;  // consume the button_click
}
void hw_test2(){
  seg_or_flag test_pattern[] = {
    {128,128,LINELEN,LINELEN,pos,0xff},
    {128,128,LINELEN,LINELEN,neg,0xff},
    {128,128,254,254,cir,0xff},
    {128,128,244,244,cir,0xff},
    {128,128,234,234,cir,0xff},
    {255,255,0,0,cir,0x00},
  }; 
  seg_or_flag test_pattern2[] = {
    {128,128,LINELEN,LINELEN,pos,0x99},
    {128,128,LINELEN,LINELEN,neg,0x99},
    {128,128,254,254,cir,0x99},
    {128,128,244,244,cir,0xff},
    {128,128,234,234,cir,0x99},
    {255,255,0,0,cir,0x00},
  }; 
  int x,y;
  int radius = 8;

// Small circle test:
  while(radius < 64){
    clear_buffer(MAIN_BUFFER);
    for(x=radius;x<256-radius;x+=2*radius)
      for(y=radius;y<256-radius;y+=2*radius){
        circle(x,y,radius,MAIN_BUFFER);
      }
    while(!button_clicked){
      display_buffer(MAIN_BUFFER);
    }
    button_clicked=0;
    radius*=2;
  }

  compileSegments(test_pattern,MAIN_BUFFER,OVERWRITE);
  while(!button_clicked) display_buffer(MAIN_BUFFER);
  button_clicked = 0;

  compileSegments(test_pattern2,MAIN_BUFFER,OVERWRITE);
  while(!button_clicked) display_buffer(MAIN_BUFFER);
  button_clicked = 0;

}


int main() 
{
  int last_switch = 0;
  /* Start up the SPI interface: */
  SPIM_1_Start();
  CyDelay(10);
    
  /* Start VDACs */
  VDAC8_1_Start();
  VDAC8_2_Start();
  
  CyDelay(50);
    
  /* Initialize the shift register: */
  ShiftReg_1_Start();
  CyDelay(50);

  // Start the quadrature decoder(aka "the knob"):
  QuadDec_1_Start();
  CyDelay(50);

  // Initialize button interrupt (an interrupt routine that polls the button at 60Hz):
  button_isr_Start();
  CyDelay(50);

  /* Initialize Wave Interrupt, which triggers at the start of each sinuisoid period: */
  isr_1_StartEx(wave_started);
  CyDelay(50);

  one_pps_int_Start();
  CyGlobalIntEnable;

  // start the UART for gps communications:
  init_gps();

  //start the real-time clock component (since the system is a clock, after all)
  // When GPS is enabled, we don't call RTC_1_Start, since GPS supplies the 1 pps

  // initTime();
   

  /* initialize sysfont: */
  init_font();

  /* initialize the four letter word randomizer: */
  init_flws();

  // initialize the EEPROM for saving prefs:
  init_prefs();
    
  CyDelay(100);
  uint8 toggle_var=0;
  hw_test2();

  // The main loop:
  for(;;){
    // test for now.  Turn off the LED part way into each 1 second period:
    if(((cycle_count-phase_error) % 31250) > 2000) LED_Reg_Write(0x0);

    RTC_1_TIME_DATE *now = RTC_1_ReadTime();
    /* Now render the appropriate contents into the display buffer, based upon 
       the current display_mode.  (Note that we're wasting lots of cpu cycles in some cases,
       since the display only changes when once/second for many of the display modes. 
       That's ok, we don't have anything more important to do.
    */
    switch (display_mode){  

    case gpsDebugMode:
      renderGPSDebug(now);
      display_buffer(MAIN_BUFFER);
      break;
    
    case flwMode:
      render_flw();
      display_buffer(MAIN_BUFFER);
      break;
    
    case textMode:
      render_text_clock(now);
      display_buffer(0);
      break;
    
    case analogMode0:
    case analogMode1:
    case analogMode2:
      renderAnalogClockBuffer(now);
      break;
    
    case secondsOnly:
      renderSeconds(now);
      break;

    case pongMode:
      pong_update();
      render_pong_buffer(game_state,now);
      break;

    case pendulumMode:
      render_pendulum_buffer(now);
      break;

    case trumpMode:
      render_trump_buffer(now);
      break;


    case trump_elapsed_mode:
      render_trump_elapsed_buffer(now);
      break;

    case xmasMode:
      render_xmas_buffer(now);
      break;

    case wordClockMode:
      render_word_clock(now);
      break;

    case julianDate:
      render_julian_date(now);
      break;

    case menuMode:
      render_menu(main_menu);
      break;
    }
    display_buffer(MAIN_BUFFER);        // display whatever we put into the display buffer on the crt

    //update the  screen-saver offsets:
    ss_x_offset = (now->Min) % 5;
    ss_y_offset =(now->Min+2) % 5;
 
    if(verbose_mode){
      int elapsed = (cycle_count-last_refresh);
      char debug_str[32];
      sprintf(debug_str,"%i/%i/%i",31250/elapsed,elapsed,loops_per_frame);
      compileString(debug_str,255,230,DEBUG_BUFFER,1,OVERWRITE);
      loops_per_frame=0;
      display_buffer(DEBUG_BUFFER);
      last_refresh = cycle_count;

    }
    int switch_interval = global_prefs.prefs_data.switch_interval;
    
    if(display_mode != menuMode && switch_interval==0) display_mode = QuadDec_1_GetCounter() % nmodes;
    else main_menu.highlighted_item_index = QuadDec_1_GetCounter() % (main_menu.n_items);
    if(display_mode == menuMode) main_menu.highlighted_item_index = QuadDec_1_GetCounter() % (main_menu.n_items);
    
    if(button_clicked){
      button_clicked=0;  // consume the click
      if(display_mode==menuMode){
	dispatch_menu(main_menu.menu_number,main_menu.highlighted_item_index);
	display_mode = saved_mode;
      }
      else {
	saved_mode = display_mode;
	display_mode = menuMode;
      }
    }
    else{
      if(display_mode != menuMode && switch_interval!=0 && cycle_count-last_switch > switch_interval*31250){
	display_mode = (cycle_count / (switch_interval*31250)) % n_auto_modes;   // switch modes automatically
        last_switch=cycle_count;  
      }
    }
  }
}

/* [] END OF FILE */
	
