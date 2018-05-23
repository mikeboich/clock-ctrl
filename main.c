 
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
#include "blanking.h"
#include "QuadDec.h"
#include "DDS_0.h"
#include "DDS_1.h"

#include "fourletter.h"
#include "gps.h"
#include "ds3231.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

//#include "ViewingLocation.h"
#include "JulianDay.h"
#include "sunrise.h"

// Real time clock variables:
volatile int pps_available=0;
volatile int second_has_elapsed=0;
volatile int minute_has_elapsed=0;

typedef enum{textMode,flwMode,bubble_mode,pongMode,pendulumMode,analogMode1, secondsOnly,sunriseMode,moonriseMode,sunElevMode,moonElevMode,trump_elapsed_mode, \
	     trumpMode,wordClockMode,xmasMode,analogMode0,analogMode2,gpsDebugMode,julianDate,menuMode} clock_type;
int nmodes = 19;
int n_auto_modes=10;
int switch_modes=0;

clock_type display_mode=pendulumMode;
clock_type saved_mode; 
uint64_t last_switch = 0;

int verbose_mode = 0;

typedef enum{idle,blank_primed,drawing,last_period,hw_testing}  draw_state;  // States of the draw loop/interrupt code

uint8 current_mask=0;
volatile draw_state current_state = idle;
volatile int current_phase=0;  // phase of sin lookup machinery
volatile int times_to_loop = 0;

volatile uint64_t cycle_count=0;  // poor man's timer
volatile uint64_t last_pulse=0;   // for validating gps pps
volatile uint8_t pulse_count=0;   


int frame_toggle = 0;   // performance measurement
volatile uint64_t phase_error=0;            // difference between (cycle_count % 31250) and 1 pps edge
volatile uint64_t minute_error=0;            // difference between (cycle_count % 60*31250) and 1 minute boundary

int previous_knob=0;  // for checking to see if knob has been turned

int last_refresh=0,loops_per_frame=0;   // for performance measurement

volatile int load_ready = 0;
/* Some housekeeping/utility items that should probably be moved to another source file: */

void led_on(){
  int status = LED_Reg_Read();
  status |= 0x1;  // low order but turns LED on
  LED_Reg_Write(status);
}
void led_off(){
  int status = LED_Reg_Read();
  status &= 0x2;  // low order bit turns LED off.  Preserve bit 1, which is hi-voltage power
  LED_Reg_Write(status);
}
void power_on(){
  int status = LED_Reg_Read();
  status |= 0x2;  // set power bit
  LED_Reg_Write(status);
}
void power_off(){
  int status = LED_Reg_Read();
  status &= 0x1;  // turn power bit off
  LED_Reg_Write(status);
}


int power_status(){
  int status = LED_Reg_Read() & 0x2;
  return status;
}
time_t power_off_t = 0;

/* Define the start-of-segment interrupt routine 
   This routine loads the next 8 bit segment mask from the display list,
   and counts the number of drawing passes/segment.
*/

CY_ISR_PROTO(wave_started);  // test comment

void wave_started(){
  isr_1_ClearPending();       // clear the interrupt
  cycle_count++;

  switch(current_state){

  case hw_testing:
    //ShiftReg_1_WriteData(0xff);    // allows us to program the DAC and let it display
    //DDS_1_SetPhase(128);  // hardwire to cos for the moment ***
    beam_on_now();                  // set the z signal to high
    break;

  case idle:
    //ShiftReg_1_WriteData(0x0);      // nothing happening.  Keep display blanked.
    //beam_off_now();  // nothing happening.  Keep display blanked.
    break;

  case last_period:
    //ShiftReg_1_WriteData(0x0);  // next period is blanked
   // beam_off_now();
    current_state = idle;
    break;

  case blank_primed:
    current_state = drawing;
    strobe_LDAC();     // causes previous programming of DAC to take effect
    break;
    
  case drawing:
    times_to_loop -= 1;         // decrement loop counter
    if(times_to_loop==0){
      //ShiftReg_1_WriteData(0x0);  // blank on the next cycle
      current_state = last_period;
    }
    else {
      //ShiftReg_1_WriteData(current_mask);
    }
    break;
  }
}

CY_ISR_PROTO(dds_load_ready);  // test comment

void dds_load_ready(){
  timer_isr_ClearPending();       // clear the interrupt
  load_ready = 1;
}

void old_adjust_phase(){
    static int dds1_phase = 0;
    static int increment = 1;
    
    load_ready = 0;
    dds1_phase += increment;
    if(dds1_phase >= 255){
        increment = -1;
        LED_Reg_Write(!LED_Reg_Read());
    }
    
    else if(dds1_phase <= 0){
        increment = 1;
        LED_Reg_Write(!LED_Reg_Read());
    }
    DDS_1_SetPhase(dds1_phase);
}

void adjust_phase()
{   
    //uint8 status = CyEnterCriticalSection();
    load_ready = 0;
    static uint8 counter = 0;   //use static to keep  recorded value
    static int8 increment = 1;  //use static to keep  recorded value
    int tmp;
    uint8 Phase = counter;
    tmp = QuadDec_Read(); 
    
    char msg[32];
    sprintf(msg, "Decoder = %i\n\r",tmp);
    SW_Tx_UART_1_PutString(msg);
    DDS_1_SetPhase(tmp % 256); // [0-256] <-> [0-2xPI]
    
    //move phase back-forward between 0 and NSteps 
    if (counter >= 255) counter=0; else    //go backwards
    if (counter <= 0) increment = 1;            //go forward
    //if(counter == 255) counter = 0;
    
    counter += increment;
    //CyExitCriticalSection(status);
}

void adjust_freq(){
    double x_freqs[] = {20000.0, 30000.0,40000.0};
    double y_freqs[] = {10000.0, 20000.0,30000.0, 40000.0};
    static long long cycles = 0;
    
    
    int nx = 3;
    int ny = 4;
    static int xindex=0;
    static int yindex=0;
    
    cycles+=1;
    if(cycles % 360 == 0){
        DDS_0_SetFrequency(x_freqs[xindex]/2.0);
        DDS_1_SetFrequency(y_freqs[yindex]/2.0);
        xindex = (xindex + 1) % nx;
        yindex = (yindex + 1) % ny;
    }
}

// utility function to decide if it's time to auto-switch display modes:
void autocheck(unsigned long period){
    if(cycle_count - last_switch > period*31250){
        switch_modes = 1;
    }
}

void renderDebugInfo(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
  char pe[64],me[64];

  char time_string[32];
  char day_of_week_string[12];
  char uptime_string[64];
  char rtc_string[15];
  char esn_string[32];
  char ds3231_string[32];
  char gps_string[32];

  int seconds = utc_bdt->tm_sec;
  int minutes = utc_bdt->tm_min;
  int hours = utc_bdt->tm_hour;

  int y = 220;
         
  //  sprintf(rtc_string,"RTC: %i:%02i:%02i",hours,minutes,seconds);
  //  compileString(rtc_string,255,y,MAIN_BUFFER,1,OVERWRITE); 
  //  y -= 36;
  clear_buffer(MAIN_BUFFER);
  time_t ds_time = get_DS3231_time();
  struct tm ds_tm = *gmtime(&ds_time);

  strftime(ds3231_string,32,"DS3231: %H:%M:%S",&ds_tm);
  compileString(ds3231_string,255,y,MAIN_BUFFER,1,APPEND);
  y-=36;

  time_t gps_time = rmc_sentence_to_unix_time(sentence);
  struct tm gps_tm = *gmtime(&gps_time);
  strftime(gps_string,32,"GPS: %H:%M:%S",&gps_tm);
  compileString(gps_string,255,y,MAIN_BUFFER,1,APPEND);
  y-=36;

  sprintf(pe,"phase error: %Lu",phase_error);
  compileString(pe,255,y,MAIN_BUFFER,1,APPEND);
  //  sprintf(uptime_string,"up %f days",cycle_count / (31250*86400.0));
  //  compileString(uptime_string,255,y,MAIN_BUFFER,1,APPEND);
  y -= 36;


  sprintf(esn_string,"Lat: %f",get_lat_or_long(0));
  compileString(esn_string,255,y,MAIN_BUFFER,1,APPEND);
  y -= 36;
  sprintf(esn_string,"Lon: %f",get_lat_or_long(1));
  compileString(esn_string,255,y,MAIN_BUFFER,1,APPEND);
  y-=36;

  if(power_off_t)
    sprintf(esn_string,"Sleep in %ldm, %lds", (power_off_t-now)/60, (power_off_t-now)%60);
  else sprintf(esn_string,"No sleep scheduled");
  compileString(esn_string,255,y,MAIN_BUFFER,1,APPEND);
 
  //  sprintf(pe,"minute err: %Lu",minute_error);
  //  compileString(pe,255,128-32,MAIN_BUFFER,1,APPEND);

  // autoswitch test:
 autocheck(4);

}

// Show a four letter word:
void render_flw(){
  char *rw;
  static uint64_t lastUpdate=0;
    
  if(cycle_count-lastUpdate > 0){  // one second update interval..
    rw = random_word();
    //rw = next_word();  // uncomment this line to have sequential, rather than random words
    compileString(rw,255,88,MAIN_BUFFER,5,OVERWRITE);
    lastUpdate = cycle_count;
        
  }
}
// Experimental animation of character segments:
#define MIN_COORD 4
#define MAX_COORD 248
//#define NUM_BUBBLES 64


int bubble_vx[BUF_ENTRIES], bubble_vy[BUF_ENTRIES];

void init_bubbles(seg_or_flag *s){
    int i;
    int velocities[] = {-2,-1,1,2};
    
    i=0;
    while(s->flag != 255){     
        bubble_vx[i] = velocities[rand() % 4]; 
        bubble_vy[i] = velocities[rand() % 4];
        i += 1;
        s++;
    } 
}
void reverse_velocities(seg_or_flag *s){
    int i = 0;
    while(s->flag !=255){
        bubble_vx[i] = -bubble_vx[i];
        bubble_vy[i] = -bubble_vy[i];
        i += 1;
        s++;
    }
}
void check_v(uint coord,int *v){
    if((coord < MIN_COORD  && *v < 0) || (coord>MAX_COORD && *v > 0))
      *v = -*v;
}
void bounce_bubbles(seg_or_flag *s){
     
    int which_bub = 0;
    while(s->flag != 255){
        check_v(s->seg_data.x_offset,&bubble_vx[which_bub]);
        check_v(s->seg_data.y_offset,&bubble_vy[which_bub]);
        s->seg_data.x_offset += bubble_vx[which_bub];
        s->seg_data.y_offset += bubble_vy[which_bub];
        
        which_bub += 1;
        s++;
    }
}

#define HR_HAND_WIDTH 8
#define HR_HAND_LENGTH 60
#define MIN_HAND_WIDTH 4
#define MIN_HAND_LENGTH 100
#define SEC_HAND_LENGTH 124

void drawClockHands(int h, int m, int s){
  if(h > 11) h -= 12;    // hours > 12 folded into 0-11  
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

void renderAnalogClockBuffer(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
  int i;
  float angle=0.0;
  static char *nums[12] = {"12","1","2","3","4","5","6","7","8","9","10","11"};
  seg_or_flag face[] = {{128,128,255,255,cir,0xff},
			{128,128,8,8,cir,0xff},
			// {0,0,255,0,pos,0xff},
			{.flag=0xff}};
  compileSegments(face,MAIN_BUFFER,OVERWRITE);
  compileString("12",112,216,MAIN_BUFFER,1,APPEND);
  compileString("6",120,20,MAIN_BUFFER,1,APPEND);
  compileString("3",220,120,MAIN_BUFFER,1,APPEND);
  compileString("9",20,120,MAIN_BUFFER,1,APPEND);
    
  drawClockHands(local_bdt->tm_hour,local_bdt->tm_min,local_bdt->tm_sec);

  if(display_mode == analogMode0){
    // experimental one revoultion/second widget:
    float x = 128.0 + (SEC_HAND_LENGTH-4)*sin(2*M_PI*(cycle_count-phase_error)/31250.0);
    float y = 128.0 + (SEC_HAND_LENGTH-4)*cos(2*M_PI*(cycle_count-phase_error)/31250.0);
    circle(x,y,16,MAIN_BUFFER);
  }

  autocheck(4);
}

asm (".global _scanf_float");       // forces floating point formatting code to load
// for displaying things like "400 days of Trump to go!" or "332 shopping days till Christmas!":
// the 
void countdown_to_event(time_t now, time_t  event_time, char *caption0, char *caption1){
  time_t current_time;
  struct tm tm_now;
  double seconds_remaining;
  double days_remaining;
  char seconds_string[64] = "";
    
  seconds_remaining = difftime(event_time,now);
  days_remaining = fabs(seconds_remaining/86400.0);  
    
    
  sprintf(seconds_string,"%.0f",days_remaining);
    
  compileString(seconds_string,255,140,MAIN_BUFFER,3,OVERWRITE);
  compileString(caption0,255,90,MAIN_BUFFER,1,APPEND);
  compileString(caption1,255,40,MAIN_BUFFER,1,APPEND);
}

void render_trump_elapsed_buffer(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
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
  autocheck(5);
}

void render_trump_buffer(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
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
  autocheck(5);
}

void render_xmas_buffer(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
  time_t end_time;
  struct tm xmas_time;
    
  xmas_time.tm_year = local_bdt->tm_year;
  xmas_time.tm_mon = 12-1;   // months are 0..11 rather than 1..12!
  xmas_time.tm_mday = 25;
  xmas_time.tm_hour = 0;  //  midnight local time
  xmas_time.tm_min = 0;
  xmas_time.tm_sec = 0;
  if(local_bdt->tm_mon == 11 && local_bdt->tm_mday > 25){
    xmas_time.tm_year += 1;
  }
  end_time = mktime(&xmas_time);
    
  countdown_to_event(now,end_time,"Shopping Days","until Christmas");
  autocheck(5);
}

void render_day_num_buffer(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
  time_t start_time;
  struct tm start_of_year;
  char str_buf[64];
  int y = 205;
    
  start_of_year.tm_year = local_bdt->tm_year;
  start_of_year.tm_mon = 1-1;   // months are 0..11 rather than 1..12!
  start_of_year.tm_mday = 1;
  start_of_year.tm_hour = 0;  //  midnight local time
  start_of_year.tm_min = 0;
  start_of_year.tm_sec = 0;
  start_time = mktime(&start_of_year);

  int day_number = (now-start_time)/86400 + 1;

  compileString("Day",255,y,MAIN_BUFFER,2,OVERWRITE);
  y -= 96;

  sprintf(str_buf,"%d",day_number);
  compileString(str_buf,255,y,MAIN_BUFFER,3,APPEND);
  y-= 72;

  sprintf(str_buf,"of %d",local_bdt->tm_year+1900);
  compileString(str_buf,255,y,MAIN_BUFFER,2,APPEND);
  autocheck(5);
  
}

double julian_date(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
  double julian_day = (now/86400.0) + 2440587.5;
  return julian_day; //  - 2400000.5;
}
// renders the Julian date
void render_julian_date(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
  double jd = julian_date(now,local_bdt,utc_bdt);
  char jd_str[32];
    
  sprintf(jd_str,"Julian Date:");
  compileString(jd_str,255,128+32,MAIN_BUFFER,1,OVERWRITE);
  sprintf(jd_str,"%.5lf",jd);
  compileString(jd_str,255,128-32,MAIN_BUFFER,1,APPEND);
  autocheck(3);
}

void render_word_clock(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
  char *strs[] = {"o'clock","b"};
  char *hour_strings[] = {"twelve","one","two","three","four","five","six","seven","eight","nine","ten","eleven"};
  char *minute_strings[] = {"not-used","five","ten","a quarter","twenty","twenty-five"};
  char past_or_until[8];
  int exact=0;
    
  char time_string[3][64];
    
  if(local_bdt->tm_min > 57 || local_bdt->tm_min < 3){
    if(local_bdt->tm_min==0)
      sprintf(time_string[0],"It's exactly");
    else
      sprintf(time_string[0],"It's about");
    compileString(time_string[0],255,160,MAIN_BUFFER,2,OVERWRITE);
    int the_hour = local_bdt->tm_min > 56 ? local_bdt->tm_hour+1 : local_bdt->tm_hour;
    sprintf(time_string[0],"%s ",hour_strings[the_hour % 12]);
    compileString(time_string[0],255,108,MAIN_BUFFER,2,APPEND);
    sprintf(time_string[0],"o'clock");
    compileString(time_string[0],255,50,MAIN_BUFFER,2,APPEND);
    return;
  }
    
  if(local_bdt->tm_min >=3 && local_bdt->tm_min <=57){
    if(local_bdt->tm_min > 27 && local_bdt->tm_min < 33){
      if(local_bdt->tm_min ==30)
	compileString("It's exactly",255,150,MAIN_BUFFER,2,OVERWRITE);
      else
	compileString("It's about",255,150,MAIN_BUFFER,2,OVERWRITE);
                
      compileString("half past",255,100,MAIN_BUFFER,2,APPEND);
      sprintf(time_string[0],"%s",hour_strings[ local_bdt->tm_hour % 12]);
      compileString(time_string[0],255,50,MAIN_BUFFER,2,APPEND);
      return;
    }
    else{
      // round to nearest 5 minutes:
      int approx_minute = 5*(local_bdt->tm_min / 5);
      int past_until_index;
      if((local_bdt->tm_min - approx_minute) > 2){
	approx_minute += 5;
      }
      exact = (approx_minute ==  local_bdt->tm_min);
            
      if(exact)
	compileString("It's exactly",255,200,MAIN_BUFFER,2,OVERWRITE);
      else
	compileString("It's about",255,200,MAIN_BUFFER,2,OVERWRITE);
      if(local_bdt->tm_min <= 27){
	past_until_index = approx_minute / 5;
	sprintf(time_string[0],"%s",minute_strings[past_until_index]);
	compileString(time_string[0],255,150,MAIN_BUFFER,2,APPEND);
	sprintf(time_string[0],"past");
	compileString(time_string[0],255,100,MAIN_BUFFER,2,APPEND);
	sprintf(time_string[0],"%s",hour_strings[local_bdt->tm_hour % 12]);
	compileString(time_string[0],255,50,MAIN_BUFFER,2,APPEND);
          
      }
      if(local_bdt->tm_min >= 33){
	approx_minute = 60-approx_minute;
	past_until_index = approx_minute / 5;
	sprintf(time_string[0],"%s",minute_strings[(approx_minute/5)]);
	compileString(time_string[0],255,150,MAIN_BUFFER,2,APPEND);                
	sprintf(time_string[0],"'til");
	compileString(time_string[0],255,100,MAIN_BUFFER,2,APPEND);                
	sprintf(time_string[0],"%s",hour_strings[((local_bdt->tm_hour+1)) % 12]);
	compileString(time_string[0],255,50,MAIN_BUFFER,2,APPEND);                
      }
    }
  } 
  autocheck(3);
}

/* ************* Pong Game ************* */
#define PADDLE_HEIGHT 24
#define PADDLE_WIDTH   8
#define PONG_TOP 250
#define PONG_BOTTOM 4
#define PONG_LEFT PADDLE_WIDTH
#define PONG_RIGHT 255-PADDLE_WIDTH
#define PADDLE_MIN PONG_BOTTOM+(PADDLE_HEIGHT/2)
#define PADDLE_MAX PONG_TOP-(PADDLE_HEIGHT/2)
#define PADDLE_STEP 4
#define MAX_Y_VELOCITY 9

typedef struct{
  uint64_t celebrating;    //  zero for normal mode, end-of-celebration time if nonzero
  int paddle_position[2]; 
  int puck_velocity[2];
  int puck_position[2];
  int score[2];
} pong_state;

pong_state game_state = {
  .celebrating = 0,
  .paddle_position = {96,140},
  .puck_velocity = {4,0},
  .puck_position = {128,128},
  .score = {0,0}};

#define CELEB_DURATION 40000     // flash some decoration on screen for just over 1 second after a score

void start_celebration(){
  game_state.celebrating = cycle_count + CELEB_DURATION;
}

void end_celebration(){
  game_state.celebrating = 0;
}

//returns which edge puck has struck, or zero otherwise:
// left = 1, right = 2, top = 3, bottom = 4
int puck_at_edge(){
  if(game_state.puck_position[0] <= PONG_LEFT){
    return(1);
    // doesn't belong here: start_celebration();
  }
  if(game_state.puck_position[0] >= PONG_RIGHT){
    return(2);
    // doesn't belong here: start_celebration();
  }
  if(game_state.puck_position[1] <= PONG_BOTTOM) return(3);
  if(game_state.puck_position[1 ] >= PONG_TOP) return(4);

  return(0);
       
}

int puck_dest(){
  float delta_x = game_state.puck_velocity[0] < 0 ? game_state.puck_position[0]-PONG_LEFT : \
    PONG_RIGHT - game_state.puck_position[0];
  float delta_t = fabs(delta_x/game_state.puck_velocity[0]);  //this many ticks to reach  edge
  float y_intercept = game_state.puck_position[1] + delta_t * game_state.puck_velocity[1];
  while(y_intercept < PONG_BOTTOM || y_intercept > PONG_TOP){
    if(y_intercept < PONG_BOTTOM) y_intercept = 2*PONG_BOTTOM - y_intercept;
    if(y_intercept > PONG_TOP) y_intercept = 2*PONG_TOP - y_intercept;
    
  }
  return (int) y_intercept;
}

int miss_zone(){     // find a paddle position that allows us to intentionally miss
  int dst = puck_dest();
  int result;
  if(dst <= PADDLE_HEIGHT){         // when they go low, we go high
    result = 2*PADDLE_HEIGHT + 4;
  }
  else if(dst > PONG_TOP - PADDLE_HEIGHT-2){  // when they go high, we go low
    result = PONG_TOP - 2 * PADDLE_HEIGHT - 4;
  }
  else{
    result = dst + PADDLE_HEIGHT + 4;  // just go high when there's room 
  }    
  return result;
}


void constrain(int *x, int xmin, int xmax){
  if(*x>xmax) *x = xmax;
  if(*x < xmin) *x = xmin;
}

#define min(x,y) x<y ? x : y

void update_paddles(int target_offset){
  int player;
  RTC_1_TIME_DATE *the_time;
  the_time = RTC_1_ReadTime();
  int should_miss[2]; //set to 1 if we want that player to miss
  int y_target;
  
  should_miss[1] = ((the_time->Min ==59 && the_time->Sec>57)) ? 1 :0;
  should_miss[0] = ((the_time->Sec > 57) && (the_time->Min != 59)) ? 1 : 0;
     
  player = game_state.puck_velocity[0] < 0 ? 0 : 1;
  if(should_miss[player]){
    y_target = miss_zone();
  }
  else{
    y_target = puck_dest() - target_offset;
  }
  int y_error = abs(y_target - game_state.paddle_position[player]);

  if(game_state.paddle_position[player] < y_target)
    game_state.paddle_position[player] += min(y_error,PADDLE_STEP );
  else game_state.paddle_position[player] -= min(y_error,PADDLE_STEP);

  constrain(&game_state.paddle_position[player],PADDLE_MIN,PADDLE_MAX);

}

// returns the new y velocity for the puck if it hit a paddle, and 0 otherwise
int puck_hit_paddle(int *new_velocity){
  int which_paddle;
  int did_hit=0;
  int offset;
    
  if(game_state.puck_velocity[0]<0 && (game_state.puck_position[0]-PONG_LEFT) <= (-game_state.puck_velocity[0]))
    which_paddle = 0;
  else if(game_state.puck_velocity[0]>0 && ((PONG_RIGHT) - game_state.puck_position[0]) <= (game_state.puck_velocity[0]))
    which_paddle=1;
  else return 0;
  
  offset = puck_dest()-game_state.paddle_position[which_paddle];
        
  if(abs(offset) > PADDLE_HEIGHT/2) return 0;  // we missed

  *new_velocity = game_state.puck_velocity[1] + offset;  // hitting off center of paddle imparts english
  constrain(new_velocity,-MAX_Y_VELOCITY,MAX_Y_VELOCITY);
  return 1;
}

void pong_update(){
  int dim;
  static int target_offset = 0;
  if(!game_state.celebrating){
    for(dim=0;dim<2;dim++){
      game_state.puck_position[dim] += game_state.puck_velocity[dim];  // move the puck
    }
    constrain(&game_state.puck_position[0],PONG_LEFT,PONG_RIGHT);
    constrain(&game_state.puck_position[1],PONG_BOTTOM,PONG_TOP);
    
    update_paddles(target_offset);
    int new_y_velocity;
    int hit_paddle = puck_hit_paddle(&new_y_velocity);
        
    if(hit_paddle){
      game_state.puck_velocity[1] = new_y_velocity;
      game_state.puck_velocity[0] = -game_state.puck_velocity[0];
      target_offset = (rand() % 7) - 3;
    }
    int which_edge = puck_at_edge();
    if(which_edge  && !hit_paddle){
      if((which_edge==1 || which_edge==2) && !hit_paddle ){
	// puck is  exiting the playing area 
	start_celebration();
	game_state.puck_velocity[0] = -game_state.puck_velocity[0];
	game_state.puck_velocity[1] = 0;
            
      }
      if(which_edge == 3 || which_edge==4){  // hit top or bottom edge. reverse y velocity:
	game_state.puck_velocity[1] = -game_state.puck_velocity[1]; 
      }
    } 
  }    
  else{
    if(cycle_count > game_state.celebrating)
      end_celebration();
  }
}

void draw_paddles(pong_state the_state){
  int y;
  // draw the left paddle
  for(y=the_state.paddle_position[0]-(PADDLE_HEIGHT/2);y<the_state.paddle_position[0]+(PADDLE_HEIGHT/2)+1;y++) \
    line(0,y,PADDLE_WIDTH,y,MAIN_BUFFER);
  // draw the right paddle:
  for(y=the_state.paddle_position[1]-(PADDLE_HEIGHT/2);y<the_state.paddle_position[1]+(PADDLE_HEIGHT/2)+1;y++) \
    line(255-PADDLE_WIDTH,y,255,y,MAIN_BUFFER);
}


void draw_puck(pong_state the_state){
  int x,y;
  x = the_state.puck_position[0];
  for(y=the_state.puck_position[1]-2;y<the_state.puck_position[1]+3;y++)
    line(x-2,y,x+2,y,MAIN_BUFFER);

}
void draw_celeb(pong_state the_state){
  int x,y,r;
  x = the_state.puck_position[0];
  y = the_state.puck_position[1];
  for(r=2; r<32; r+=8){
    circle(x,y,r,MAIN_BUFFER);
  }
}

void draw_center_line(pong_state the_state){
  int x,y;
  x=128;
  for(y=PONG_TOP;y>0;y-=32){
    line(128,y,128,y-16,MAIN_BUFFER);   
  }  
}

void draw_scores(pong_state the_state, struct tm *local_bdt){  // draw the hours and minutes as two scores:
  char time_str[32];
  int the_hour = local_bdt->tm_hour;
  int the_minute = local_bdt->tm_min;
  sprintf(time_str,"%02i",the_hour);
  compileString(time_str,36,200,MAIN_BUFFER,2,APPEND);
    
  sprintf(time_str,"%02i",the_minute);
  compileString(time_str,160,200,MAIN_BUFFER,2,APPEND); 
}
void draw_tick(int seconds){  // draw a seconds tick
  float second_angle = ((seconds/60.0))*M_PI*2.0;
  float x0 = 128 + 60*sin(second_angle);
  float y0 = 128 + 60*cos(second_angle);
  x0 = 128;
  y0 = 128;
  float x1 = 128 + 64*sin(second_angle);
  float y1 = 132 + 64*cos(second_angle);
  circle(128,132,8,MAIN_BUFFER);
  circle(128,132,80,MAIN_BUFFER);
    
  line(x0,y0,x1,y1,MAIN_BUFFER);
    
}
void render_pong_buffer(pong_state the_state, time_t now, struct tm *local_bdt, struct tm *utc_bdt){
  int x,y;
  
  clear_buffer(MAIN_BUFFER);
  //draw_tick(local_bdt->tm_sec);
  draw_paddles(the_state);
  if(!the_state.celebrating) draw_puck(the_state);
  draw_center_line(the_state);
  draw_scores(the_state,local_bdt);

  if(the_state.celebrating && cycle_count % 6000 > 3000)
    draw_celeb(the_state);
    
  if(cycle_count-last_switch > 6*31250){
  switch_modes=1;
  }
    
//  if(cycle_count-last_switch > 3*31250){
//    display_mode = (display_mode+1) % n_auto_modes;
//    last_switch = cycle_count;
//}
}
void render_text_clock(time_t now,struct tm *local_bdt, struct tm *utc_bdt);

#define BOUNCE_PERIOD 140

// Animate individual segments of the characters in the word-clock display:
void render_bubble_buffer(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
    static int step_number=0;

    if(minute_has_elapsed){
        step_number = 0;
        render_word_clock(now,local_bdt,utc_bdt);
        minute_has_elapsed = 0;
    }
    if(step_number == 0){
        init_bubbles(seg_buffer[MAIN_BUFFER]);
        bounce_bubbles(seg_buffer[MAIN_BUFFER]);
    }
    else if(step_number < BOUNCE_PERIOD){
        bounce_bubbles(seg_buffer[MAIN_BUFFER]);
    }
    else if(step_number < 2*BOUNCE_PERIOD){
        if(step_number==BOUNCE_PERIOD){  // time to reverse
            reverse_velocities(seg_buffer[MAIN_BUFFER]);
        }
        bounce_bubbles(seg_buffer[MAIN_BUFFER]);
    }
    else if( step_number < 3*BOUNCE_PERIOD){
    }
    else{
      step_number = -1;  
      render_word_clock(now,local_bdt,utc_bdt);
      // autoswitch:
      switch_modes=1;
    }
    step_number += 1;
}
#define FLW_ANIM_PERIOD 160
void render_flw_animated_buffer(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
    static int step_number=0;

    if(step_number==0 ){
        render_flw();
    }
    if(step_number == 0){
        init_bubbles(seg_buffer[MAIN_BUFFER]);
        for(step_number=0;step_number<FLW_ANIM_PERIOD;step_number++)
          bounce_bubbles(seg_buffer[MAIN_BUFFER]);
        reverse_velocities(seg_buffer[MAIN_BUFFER]);
    }
    //else if(step_number < FLW_ANIM_PERIOD){
        //bounce_bubbles(seg_buffer[MAIN_BUFFER]);
    //}
    else if(step_number <= 2*FLW_ANIM_PERIOD){

        bounce_bubbles(seg_buffer[MAIN_BUFFER]);
    }
    else if( step_number < 5*FLW_ANIM_PERIOD){
    }
    else{
      step_number = -1;  
      // autoswitch time:
      autocheck(5);
    }
    step_number += 1;
}
/*  Pendulum Clock *** */
void render_pendulum_buffer(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
  char sec_str[32],hr_min_string[32];
  float x,y,i;

  // render the time in seconds
  sprintf(sec_str,"%02i",local_bdt->tm_sec);
  compileString(sec_str,255,32,MAIN_BUFFER,2,OVERWRITE);

  // render the hour and minute:  
  sprintf(hr_min_string,"%02i:%02i",local_bdt->tm_hour,local_bdt->tm_min);
  compileString(hr_min_string,255,115,MAIN_BUFFER,3,APPEND);

  // render the pendulum shaft:  
  x = 128.0+200*sin(sin(2*M_PI*(cycle_count-phase_error)/31250.0)/2.5);
  y = 250.0 - 200*cos(sin(2*M_PI*(cycle_count-phase_error)/31250.0)/2.5);
  line(128,250,x,y,MAIN_BUFFER);

  //render the pendulum bob:
  for(i=32;i>0;i-=8) circle(x,y,i,MAIN_BUFFER);

  //render the point from which the pendulum swings:
  circle(128,245,8,MAIN_BUFFER);

  //auto-switch test:
  autocheck(5);
  

}

void render_text_clock(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
  char time_string[32];
  char day_of_week_string[12];
  char date_string[15];

  int seconds = local_bdt->tm_sec;
  int minutes = local_bdt->tm_min;
  int hours = local_bdt->tm_hour;
    
  int day_of_week = local_bdt->tm_wday;
  int month = local_bdt->tm_mon;
  int day_of_month = local_bdt->tm_mday;
  int year = local_bdt->tm_year+1900;
        
  sprintf(time_string,"%i:%02i:%02i",hours,minutes,seconds);
  compileString(time_string,255,46,MAIN_BUFFER,3,OVERWRITE);

  sprintf(date_string,"%s %i, %i",month_names[month],day_of_month,year);
 
  compileString(date_string,255,142,MAIN_BUFFER,1,APPEND);
     
  char dw[12];
  sprintf(dw,"%s",day_names[day_of_week]);
  compileString(dw,255,202,MAIN_BUFFER,2,APPEND);

// autoswitch logic, since this is an auto-mode:
  autocheck(5);
}

void renderSeconds(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
  char sec_str[4];
  char hour_min_str[8];
  char day_of_week_str[16];
    
  sprintf(hour_min_str,"%02i:%02i",local_bdt->tm_hour ,local_bdt->tm_min);
  sprintf(sec_str,"%02i",local_bdt->tm_sec);
  sprintf(day_of_week_str,"%s",day_names[local_bdt->tm_wday]);
  compileString(sec_str,255,10,MAIN_BUFFER,2,OVERWRITE);
  compileString(hour_min_str,255,85,MAIN_BUFFER,4,APPEND);
  compileString(day_of_week_str,255,205,MAIN_BUFFER,2,APPEND);
  autocheck(5);
}
int inBounds(float x, float lower, float upper){
  if(x <= upper && x >= lower) return 1;
  else return 0;
}
#define SUN_SIZE 64

// Sun elevation diagram, as inspired by SGITeach:
#define LEFT_MARGIN 8
#define RIGHT_MARGIN 248

void renderSunOrMoonElev(time_t now,struct tm *local_bdt, struct tm *utc_bdt,int zeroForSunOneForMoon){
  int x,y;
  static seg_or_flag axes[] = {
    {128,120,0,252 ,neg ,0x0ff},   // y-axis creates a line from 128,8 to 128,248
    {128,8,240,0,neg,0x0ff},      // x-axis creates a line from 8,8 to 248,8
    {255,255,0,0,cir,0x00},
  };
  time_t today = midnightInTimeZone(now,global_prefs.prefs_data.utc_offset);
  static time_t last_calcs = 0;
  time_t t;

  static int time_to_y[24][2];
  static int y_at_rise[2], y_at_set[2], x_at_rise[2], x_at_set[2];
  static double rise_elev, set_elev;
  static time_t rise_time[2],set_time[2];

    
  // render axes:
  compileSegments(axes,MAIN_BUFFER,OVERWRITE);
  //line(0,8,255,8,MAIN_BUFFER);   // horiz axis
    

  for(x=8;x<=240;x+=10){         // horiz axis tick marks
    line(x,0,x,16,MAIN_BUFFER);  
  }
    
  //line(128,0,128,240,MAIN_BUFFER);
  for(y=18;y<=248;y+=26){    // vertical axis tick marks
    line(120,y+8,136,y+8,MAIN_BUFFER);
  }

// draw a dotted line denoting the current time:
   float day_fraction = local_bdt->tm_hour/24.0 + local_bdt->tm_min/1440.0;
  int x_offset = LEFT_MARGIN + (day_fraction)*(RIGHT_MARGIN-LEFT_MARGIN);
  vertical_dashed_line(x_offset,0,x_offset,180,MAIN_BUFFER);

// calc elevations if necessary:
  double elev;
  struct location my_location;
  init_location(&my_location);
    
  int i;
  if(today != last_calcs){
    for(i=0;i<2;i++){
      rise_time[i] = calcSunOrMoonRiseForDate(today,1,i+1,my_location);
      if(i==0)
	calcSolarAzimuth(NULL, &rise_elev, NULL, NULL, rise_time[i], my_location);
      else
	calcLunarAzimuth(NULL, &rise_elev, NULL, NULL, NULL, rise_time[i], my_location);
      y_at_rise[i] = 2.6*rise_elev;
      x_at_rise[i] = 8 + (rise_time[i]-today)/360;
      rise_time[i] += global_prefs.prefs_data.utc_offset*3600;
      set_time[i] = calcSunOrMoonRiseForDate(today,2,i+1,my_location);
      if(i==0)
	calcSolarAzimuth(NULL, &set_elev, NULL, NULL, set_time[i], my_location);
      else
	calcLunarAzimuth(NULL, &set_elev, NULL, NULL, NULL, set_time[i], my_location);
            
      y_at_set[i] = 2.6*set_elev;
      x_at_set[i] = 8 + (set_time[i]-today)/360;
      set_time[i] += global_prefs.prefs_data.utc_offset*3600;
            
      int index=0;
      for(t=today;t<today+86400;t+=3600){
	if(i==0)
	  calcSolarAzimuth(NULL, &elev, NULL, NULL, t, my_location);
	else
	  calcLunarAzimuth(NULL, &elev, NULL, NULL, NULL, t, my_location);
	time_to_y[index][i] = 2.6*elev;
	index += 1;
      }
      last_calcs = today;
    }
  }
  else{  // calcs have already been done for today.  Just use the cached values:
    int hour;
    struct tm bdt;
    char event_str[64];

    if(zeroForSunOneForMoon==0){
      compileString("Sunrise",16,220,MAIN_BUFFER,1,APPEND);
      compileString("Sunset",154,220,MAIN_BUFFER,1,APPEND);
    }
    else{
      compileString("Moonrise",16,220,MAIN_BUFFER,1,APPEND);
      compileString("Moonset",154,220,MAIN_BUFFER,1,APPEND);
    }
        
    bdt = *gmtime(&rise_time[zeroForSunOneForMoon]);
    strftime(event_str,sizeof(event_str),"%l:%M %p",&bdt);
    compileString(event_str,0,190,MAIN_BUFFER,1,APPEND);
    bdt = *gmtime(&set_time[zeroForSunOneForMoon]);
    strftime(event_str,sizeof(event_str),"%l:%M %p",&bdt);
    compileString(event_str,138,190,MAIN_BUFFER,1,APPEND);

    for(hour=0;hour<24;hour++){
      y = time_to_y[hour][zeroForSunOneForMoon];
      if(y > 0){
	circle(10*hour+8,y+8,8,MAIN_BUFFER); 
      }
    }
    circle(x_at_rise[zeroForSunOneForMoon],y_at_rise[zeroForSunOneForMoon]+8,8,MAIN_BUFFER);
    circle(x_at_set[zeroForSunOneForMoon],y_at_set[zeroForSunOneForMoon]+8,8,MAIN_BUFFER);
  }
  
}

// Sun elevation diagram, as inspired by SGITeach:
void renderSunElev(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
  renderSunOrMoonElev(now,local_bdt,utc_bdt,0);
  autocheck(6);
}


// Moon elevation diagram, as inspired by SGITeach:
void renderMoonElev(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
  renderSunOrMoonElev(now,local_bdt,utc_bdt,1);
  autocheck(6);
}
// This (pretty ugly) routine serves for both sunset/sunrise and moonset/moonrise
void renderSR2(time_t now,struct tm *local_bdt, struct tm *utc_bdt){
  static time_t date_for_calcs = 0;
  static time_t sunrise_time, sunset_time,moonrise_time,moonset_time;
  static double moon_fullness=0.0;
  char fullness_str[255];
  char event_str[64];
  struct tm bdt;
  struct location my_location;  // this will move to prefs and/or we'll get it from GPS
 
  static seg_or_flag sun[] = {
    {128,0,SUN_SIZE,SUN_SIZE,cir,0x0ff},   
    {255,255,0,0,cir,0x00},
  };


  static seg_or_flag moon[] = {
    {128,0,127,127,cir,0xff},
    {144,0,38,42,cir,0xff},
    {106,10,12,14,cir,0xff},
    {114,26,14,12,cir,0xff},
    {140,38,24,20,cir,0xff},
    {255,255,0,0,cir,0x00},
  };


  static uint64_t next_animation_time=0;  // allows us to keep tracj and calc rises and sets once/day
  static int sun_y=0;
  const int animation_period = 1024;
  int oneForSun = (display_mode==sunriseMode) ? 1 : 2;

  static int animation_step = 1;
  int animation_start = 0;
  int animation_stop = 80;
  
  // advance the animation it it's time:
  if(cycle_count > next_animation_time){
    offsetSegments(sun,0,animation_step);
    offsetSegments(moon,0,animation_step);
    next_animation_time = cycle_count + animation_period;
    sun_y += animation_step;
    if(sun_y == animation_stop){
      animation_step = -1;
    }
    else if(sun_y == 0){
      animation_step = 1;
    }
  }

  clear_buffer(MAIN_BUFFER);
  //insetSegments(sun,16,16);
  if(oneForSun==1){
    float angle;
    float outset = 0.6*SUN_SIZE;
    float outset2 = 0.9*SUN_SIZE;
    compileSegments(sun,MAIN_BUFFER,APPEND);
    // draw rays

    for(angle = 0.0; angle < 2*M_PI-0.1; angle += 2*M_PI/12.0){
      float origin_x = 128 + outset*cos(angle);
      float origin_y = sun_y + outset*sin(angle);
      float end_x = 128 + outset2 * cos(angle);
      float end_y = sun_y + outset2 * sin(angle);
    
      if(inBounds(origin_x,0.0,255.0) && \
	 inBounds(origin_y,0.0,255.0) && \
	 inBounds(end_x,0.0,255.0) && \
	 inBounds(end_y, 0.0, 255.0)){
	line(origin_x,origin_y,end_x,end_y,MAIN_BUFFER);
      }
    }
  }
  else{  // draw moon features here:
    compileSegments(moon,MAIN_BUFFER,APPEND);
  }
  time_t today = midnightInTimeZone(now,global_prefs.prefs_data.utc_offset);

  if(today != date_for_calcs){
    date_for_calcs = today;
    init_location(&my_location);  // test values for now..
        
    sunrise_time = calcSunOrMoonRiseForDate(today,1,1,my_location);  // sunrise UTC
    sunrise_time += global_prefs.prefs_data.utc_offset*3600;         // cheesy offset to local time
    sunset_time = calcSunOrMoonRiseForDate(today,2,1,my_location);   // sunset UTC
    sunset_time += global_prefs.prefs_data.utc_offset*3600;          // offset to local time for display

    moonrise_time = calcSunOrMoonRiseForDate(today,1,2,my_location);   // analogous to above for moon...
    moonrise_time += global_prefs.prefs_data.utc_offset*3600;
    moonset_time = calcSunOrMoonRiseForDate(today,2,2,my_location);
    moonset_time += global_prefs.prefs_data.utc_offset*3600;
        
    // calculate fullness of moon for good measure
    calcLunarAzimuth(NULL, NULL, &moon_fullness, NULL, NULL, now,my_location);

  }
    
  if(animation_step == 1){
    bdt = oneForSun==1 ? *gmtime(&sunrise_time) : *gmtime(&moonrise_time);
    strftime(event_str,sizeof(event_str),"%l:%M %p",&bdt);
  }
  else {
    bdt = oneForSun==1 ? *gmtime(&sunset_time) : *gmtime(&moonset_time);
    strftime(event_str,sizeof(event_str),"%l:%M %p",&bdt);
  }
  compileString(event_str,255,160,MAIN_BUFFER,2,APPEND);
    
  if(oneForSun==2){
    sprintf(fullness_str,"%.0f%% full",100*moon_fullness);
    compileString(fullness_str,255,230,MAIN_BUFFER,1,APPEND);
  }
  autocheck(6);
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

/* 
   This routine, plus the interrupt routine, constitute the inner loop of the drawing machinery
   It cycles through a buffer full of arcs and lines (aka segments), and programs the display 
   hardware to show them.
   Because it runs asynchronously to the actual display, it polls to see when the hw
   is ready for new commands.  When the hardware is busy, it takes the opportunity to process
   incoming characters from the GPS Module.  (Inspired from the original Macintosh disk driver,
   that polled the serial ports during its inner loop to avoid missing characters.
*/
void display_buffer(uint8 which_buffer){
#define PI 3
    
  seg_or_flag *seg_ptr = seg_buffer[which_buffer];
  //FrameDrawReg_Write(frame_toggle);
  frame_toggle = 1 - frame_toggle;
  sync_to_60Hz();
  while(seg_ptr->seg_data.x_offset != 0xff){

    if(current_state==idle){
      uint8 int_status = CyEnterCriticalSection();
            
      set_DACfor_seg(seg_ptr,ss_x_offset,ss_y_offset);
      switch(seg_ptr->seg_data.arc_type){
      case cir:
        current_phase=0x1;   // phase register can't be written here, as drawing may still be active, so set current_phase instead
        DDS_1_SetPhase(64);
	break;
        
      case pos:
	current_phase = 0x0;
    DDS_1_SetPhase(0);
	break;
        
      case neg:
        current_phase = 0x2;
        DDS_1_SetPhase(128);
	break;
      }
#define SGI_TEACH   
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

      //ShiftReg_1_WriteData(current_mask);  // "prime" the shift register

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
  /*  RTC_1_TIME_DATE *the_time = RTC_1_ReadTime();
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
  */
}


void waitForClick(){
  while(!button_clicked);
  button_clicked=0;
}

#define LINELEN 128

void hw_test(){
  seg_or_flag test_pattern[] = {
    {128,128,220,220,cir,0xff},
    {255,255,0,0,cir,0x00},
  }; 
  seg_or_flag test_pattern2[] = {
    {128,128,128,128,cir,0xff},
    {128,128,244,244,cir,0xff},
    {255,255,0,0,cir,0x00},
  }; 
  button_clicked = 0;  
  timer_isr_StartEx(dds_load_ready);

  DDS_0_SetFrequency(31250);
  DDS_1_SetFrequency(31250);

  set_DACfor_seg(test_pattern2,0,0);
  strobe_LDAC();

  Timer_Reg_Write(0);  // turn off DDS and timers and beam
  CyDelay(1);
  DDS_1_SetPhase(64);
  Z_On_Timer_WriteCounter(16*24);
  Z_On_Timer_WritePeriod(32*24);
  Z_Off_Timer_WriteCounter(24*24);
  Z_Off_Timer_WritePeriod(32*24);

  Timer_Reg_Write(DDS_ENABLE | ON_TIMER_ENABLE | OFF_TIMER_ENABLE);

  //current_state = hw_testing;
  while(!button_clicked) {
    if(load_ready){
      adjust_phase();   
      adjust_freq();
    }
  }
    button_clicked = 0;
  beam_off_now();
  CyDelay(1000);
  beam_on_now();
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
    for(x=radius;x<256-radius+1;x+=2*radius)
      for(y=radius;y<256-radius+1;y+=2*radius){
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

  /* Start up the SPI interface: */
  SPIM_1_Start();
  CyDelay(10);

  /* Start up i2c for DS3231 clock */
 // I2C_1_Start();

 // start up the DDS phase accumulators:
  DDS_0_Start();
  DDS_1_Start();
    
  /* Start VDACs */
  VDAC8_1_Start();
  VDAC8_2_Start();
  
  CyDelay(50);
    

  // Start the quadrature decoder(aka "the knob"):
  QuadDec_Init();
  CyDelay(50);

  // Initialize button interrupt (an interrupt routine that polls the button at 60Hz):
  button_isr_Start();
  CyDelay(50);

  /* Initialize Wave Interrupt, which triggers at the start of each sinuisoid period: */
  isr_1_StartEx(wave_started);
  CyDelay(50);

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

  // with prefs initialized, we can select between gps or the internal ds3231 rtc:
  // first delay one second to allow a pps pules to arrive:
  CyDelay(1100);
    
  CyDelay(100);
  uint8 toggle_var=0;
  write_DS3231_status_reg(0x00);  //default modes, including output of 1Hz square wave

  time_t t = get_DS3231_time();
  RTC_1_TIME_DATE *psoc_now = RTC_1_ReadTime();
  unix_to_psoc(t,psoc_now);  // copy current DS3231 time to psoc RTC
  gps_pps_int_Start();
  DS3231_pps_int_Start();

  // set the power off time from the prefs value:
  if (global_prefs.prefs_data.minutes_till_sleep > MAX_TILL_SLEEP)  // this value means "never sleep"
    power_off_t = 0;
  else
    power_off_t = t + 60*global_prefs.prefs_data.minutes_till_sleep;

  SW_Tx_UART_1_StartEx(15,0);
  SW_Tx_UART_1_PutString("Hello from PSOC-land!");
  power_on();

  hw_test();
  //hw_test2();
  previous_knob = QuadDec_Read();

  // The main loop:
  for(;;){
    // Turn off the LED part way into each 1 second period:
    // do it slightly different when using DS3231 vs GPS, so that we have
    // a visual indication of which mode we're in.
    if(((cycle_count-phase_error) % 31250) > 2000 && !global_prefs.prefs_data.use_gps){
      led_off();
    }
    else if(((cycle_count-phase_error) % 31250) > 20000){
      led_off();
    }

    RTC_1_TIME_DATE *psoc_now = RTC_1_ReadTime();
    time_t now;
    now = psoc_to_unix(psoc_now);
    time_t to_local = now+global_prefs.prefs_data.utc_offset*3600;
    struct tm local_bdt = *gmtime(&to_local);  // my way of getting local time
    struct tm utc_bdt = *gmtime(&now);
    
    if(power_off_t && now > power_off_t){      // time to go to sleep?
      power_off();
      power_off_t = 0;
    }
    
    /* Now render the appropriate contents into the display buffer, based upon 
       the current display_mode.  (Note that we're wasting lots of cpu cycles in some cases,
       since the display only changes when once/second for many of the display modes. 
       That's ok, we don't have anything more important to do.
    */
    
   
    switch (display_mode){  

    case gpsDebugMode:
      renderDebugInfo(now,&local_bdt,&utc_bdt);
      break;
    
    case flwMode:
      //render_flw();
      render_flw_animated_buffer(now,&local_bdt,&utc_bdt);
      break;
    
    case textMode:
      render_text_clock(now,&local_bdt,&utc_bdt);
      break;
    
    case analogMode0:
    case analogMode1:
    case analogMode2:
      renderAnalogClockBuffer(now,&local_bdt,&utc_bdt);
      break;
    
    case secondsOnly:
      renderSeconds(now,&local_bdt,&utc_bdt);
      break;

    case sunriseMode:
      //renderSunrise(now,&local_bdt,&utc_bdt);
      renderSR2(now,&local_bdt,&utc_bdt);
      break;

    case moonriseMode:
      //renderSunrise(now,&local_bdt,&utc_bdt);
      renderSR2(now,&local_bdt,&utc_bdt);
      break;

    case sunElevMode:
      //renderSunrise(now,&local_bdt,&utc_bdt);
      renderSunElev(now,&local_bdt,&utc_bdt);
      break;

    case moonElevMode:
      //renderSunrise(now,&local_bdt,&utc_bdt);
      renderMoonElev(now,&local_bdt,&utc_bdt);
      break;

    case pongMode:
      pong_update();
      render_pong_buffer(game_state, now,&local_bdt,&utc_bdt);
      break;

    case pendulumMode:
      render_pendulum_buffer(now,&local_bdt,&utc_bdt);
      break;

    case bubble_mode:
      render_bubble_buffer(now,&local_bdt,&utc_bdt);
      break;

    case trumpMode:
      render_trump_buffer(now,&local_bdt,&utc_bdt);
      break;


    case trump_elapsed_mode:
      render_trump_elapsed_buffer(now,&local_bdt,&utc_bdt);
      break;

    case xmasMode:
      //render_xmas_buffer(now,&local_bdt,&utc_bdt);
      render_day_num_buffer(now,&local_bdt,&utc_bdt);
      break;

    case wordClockMode:
      render_word_clock(now,&local_bdt,&utc_bdt);
      break;

    case julianDate:
      render_julian_date(now,&local_bdt,&utc_bdt);
      break;

    case menuMode:
      render_menu(main_menu);
      break;
    }
    display_buffer(MAIN_BUFFER);        // display whatever we put into the display buffer on the crt

    //update the  screen-saver offsets:
    ss_x_offset = (local_bdt.tm_min) % 5;
    ss_y_offset =(local_bdt.tm_min+2) % 5;
 
    if(verbose_mode){
      int elapsed = (cycle_count-last_refresh);
      char debug_str[32];
      sprintf(debug_str,"%i/%i/%i",31250/elapsed,elapsed,loops_per_frame);
      compileString(debug_str,255,230,DEBUG_BUFFER,1,OVERWRITE);
      loops_per_frame=0;
      display_buffer(DEBUG_BUFFER);
      last_refresh = cycle_count;

    }
    int switch_interval = global_prefs.prefs_data.switch_interval;  //formerly an interval.  Now a boolean.
    int knob_position;
    
    // if not auto-switching, set mode according to knob position:
    if(display_mode != menuMode && switch_interval == 0) {
      if((knob_position=QuadDec_Read()) < 0) QuadDec_Write(knob_position + nmodes);  // wrap around
      display_mode = QuadDec_Read() % nmodes;
    }
    else {
      if((knob_position=QuadDec_Read()) < 0) QuadDec_Write(knob_position + main_menu.n_items);  // wrap around
      main_menu.highlighted_item_index = QuadDec_Read() % (main_menu.n_items);
    }
    //if(display_mode == menuMode) main_menu.highlighted_item_index = QuadDec_1_GetCounter() % (main_menu.n_items);
    
    
    if(button_clicked){
      button_clicked=0;  // consume the click
       // if the power is off, take the button click as a command to turn it on:
      if(power_status() == 0){
        power_on();
        if(global_prefs.prefs_data.minutes_till_sleep > 0 && global_prefs.prefs_data.minutes_till_sleep < MAX_TILL_SLEEP)
          power_off_t = now + 60 * global_prefs.prefs_data.minutes_till_sleep;  // reset timer
        else if(global_prefs.prefs_data.minutes_till_sleep == -1)
          power_off_t = 0;
      }
      else{
	// else if we're in menu mode, perform the selected item and return to a display mode
	if(display_mode==menuMode){
	  dispatch_menu(main_menu.menu_number,main_menu.highlighted_item_index);
	  display_mode = saved_mode;
	  QuadDec_Write(saved_mode);
	  previous_knob = saved_mode;
	}
	else {
	  // we were in a display mode.  enter menuMode:
	  saved_mode = display_mode;
	  display_mode = menuMode;
	}
      }
    }
    
    else{  // auto-switch
      if(display_mode != menuMode && switch_interval!=0 && switch_modes){
        if(local_bdt.tm_sec > 48 && local_bdt.tm_min % 2 ==1){
           display_mode = pongMode;  // so we can see the score at the end of each minute 
        }
        else{
	      display_mode = (display_mode+1) % n_auto_modes;   // switch modes automatically
        }
        switch_modes=0;
        last_switch=cycle_count; 
       }
    }
    
    // if knob has been turned, bump sleep timer and exit autoswitch:
    if (QuadDec_Read() != previous_knob){
      global_prefs.prefs_data.switch_interval = 0;   // cancels autoswitch
      if(global_prefs.prefs_data.minutes_till_sleep > 0 && global_prefs.prefs_data.minutes_till_sleep <= MAX_TILL_SLEEP)
        power_off_t = now + 60 * global_prefs.prefs_data.minutes_till_sleep;  // bump the time until sleep
      previous_knob = QuadDec_Read();
    }

  }
}
// bug fixes since last github sync:
// corrected several calls to sunrise/moonrise calcs that weren't using MidnightInTimeZone
// fixed restoration of displayMode, so that executing a menu item doesn't change display
/* [] END OF FILE */
	
       
       
