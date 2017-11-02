/*  menu.c

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
#include "max509.h"
#include "menus.h"
#include "draw.h"
#include "prefs.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "DS3231.h"

extern int cycle_count;
extern int verbose_mode;
//int button_clicked;

// Some useful strings:
char *day_names[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
char *month_names[12] = {"Jan", "Feb", "Mar", "April","May","June","July","Aug","Sep","Oct","Nov","Dec"};

menu main_menu = {.items = {"Set Time/Date","Set Locale","Autoswitch","Character Set","Test Pattern","Use GPS","Power Off"},
		  .n_items = 7,
		  .highlighted_item_index = -1,
		  .menu_number = 0};

menu *current_menu = &main_menu;



void sync_to_60Hz(){
    static int phase=0;
     if(global_prefs.prefs_data.sync_to_60Hz){
      phase = SixtyHz_Read();
      while(SixtyHz_Read() == phase);   // wait for a 60Hz edge..

    }
}

void wait_for_twist(){
 int tmp = QuadDec_1_GetCounter();
 while(tmp==QuadDec_1_GetCounter());
}

void render_menu(menu the_menu){
  compile_menu(&the_menu,MAIN_BUFFER);
  //active_menu = &the_menu;
  
}
#define MENU_Y_SPACING 32
void compile_menu(menu *the_menu, int which_buffer){
  int item_index;
  int y;
  char item_str[80] = "";
 
  y = 128+(the_menu->n_items/2)*MENU_Y_SPACING;
  clear_buffer(MAIN_BUFFER);
  for(item_index=0;item_index<the_menu->n_items;item_index++){
    item_str[0] = 0;
    if(item_index==the_menu->highlighted_item_index) strcat(item_str,"->");
    strcat(item_str,the_menu->items[item_index]);
    compileString(item_str,255,y,which_buffer,1,APPEND);
    y -= MENU_Y_SPACING;
  }
}

// copy time data into an array for easy manipulation:
void unpack_time(RTC_1_TIME_DATE *t, int a[]){
  a[0] = t->Month;
  a[1] = t->DayOfMonth;
  a[2] = t->Year;
    
  a[3] = t->Hour;
  a[4] = t->Min;
  a[5] = t->Sec;
    
  a[6] = t->DayOfWeek-1;
}

// copy array of values back into time structure:
void pack_time(RTC_1_TIME_DATE *t, int a[]){
  t->Month = a[0];
  t->DayOfMonth = a[1];
  t->Year = a[2];

  t->Hour = a[3];
  t->Min = a[4];
  t->Sec = a[5];

  t->DayOfWeek = a[6]+1;
}

struct {RTC_1_TIME_DATE the_time;
  int selected_field;
} time_set_state;

int blink_time(){
  return ((cycle_count % 31250) > 15625);
}
void compile_time_screen(int a[],int selected_field){
  int field;
  int x_pos[7] = {118-48,118,118+48,128-48,128,128+48,128-64};
  int y_pos[7] = {200,200,200,110,110,110};
  seg_or_flag segment;
  char num_buf[8];
  char format_str[7][8] = {"%02i/","%02i/","%i","%02i:","%02i:","%02i","%s"};
    
  clear_buffer(MAIN_BUFFER);
  for(field=0; field<6; field++){
    num_buf[0] = 0;     // make string null string
    sprintf(num_buf, format_str[field],a[field]);
   if(selected_field!=field || blink_time())
      compileString(num_buf,x_pos[field],y_pos[field],MAIN_BUFFER,1,APPEND);
  }
  if(selected_field!=field || blink_time())
    compileString(day_names[a[6]],255,y_pos[6],MAIN_BUFFER,1,APPEND);
}

void display_buffer(uint8 which_buffer);



void set_the_time(){
  int time_params[7];
  RTC_1_TIME_DATE psoc_time;
  time_t clock_time;
  clock_time = get_DS3231_time();

  int selected_field=0;
  int prev_counter = QuadDec_1_GetCounter();
  int field_min[7] = {1,1,2016,0,0,0,0};
  int field_max[7] = {12,31,2100,23,59,59,6};
  uint8 done=0;
  unix_to_psoc(clock_time,&psoc_time);

  unpack_time(&psoc_time,time_params);
  QuadDec_1_SetCounter(time_params[selected_field]);

  while(!done){
    compile_time_screen(time_params,selected_field);
    display_buffer(MAIN_BUFFER);
    int new_counter = QuadDec_1_GetCounter();
    if(new_counter < field_min[selected_field] || new_counter > field_max[selected_field]){
        new_counter = (new_counter < field_min[selected_field]) ? field_min[selected_field] : field_max[selected_field];
    }
    time_params[selected_field] = new_counter;
    if(button_clicked){
      button_clicked=0;  // consume the click..
      if(selected_field==6){
        struct tm utime;
        utime.tm_hour = time_params[3];
        utime.tm_min = time_params[4];
        utime.tm_sec = time_params[5];
        utime.tm_mon = time_params[0]-1;
        utime.tm_mday = time_params[1];
        utime.tm_year = time_params[2]-1900;
        utime.tm_isdst = 0;
        time_t t = mktime(&utime);
        setDS3231(t);  // set the DS3231
        RTC_1_TIME_DATE *psoc_time = RTC_1_ReadTime();
        unix_to_psoc(t,psoc_time);  //set the psoc clock
	    done=1;
      }
      else {
	    selected_field = (selected_field + 1) % 7; 
        QuadDec_1_SetCounter(time_params[selected_field]);
      }    
    }
  }  
}

void char_test(){
  compileString("abcdefghijklm",255,230,0,1,OVERWRITE);
  compileString("nopqrstuvwxyz",255,180,0,1,APPEND);
  compileString("~`!@#$%^&*()_-+={}",255,130,0,1,APPEND);
  char str4[] = {128,129,130,131,132,0};  //Japanese extended characters
  compileString(str4,255,60,0,2,APPEND);
    
  while(!button_clicked){
    display_buffer(0);   
  }
  button_clicked = 0;
  compileString("ABCDEFGHIJKLM",255,230,0,1,OVERWRITE);
  compileString("NOPQRSTUVWXYZ",255,180,0,1,APPEND);
  compileString("1234567890",255,130,0,1,APPEND);
  char str8[] = {133,134,135,136,0};   // More Japanese characters
  compileString(str8,255,60,0,2,APPEND);
    
  while(!button_clicked){
    display_buffer(0);   
  }
  button_clicked=0;
 
}

void align_screen2(){
  seg_or_flag test_pattern[] = {
    {128,128,254,254,cir,0x0ff},
    {128,254,8,8,cir,0xff},  //top
    {128,1,8,8,cir,0xff},  //bottom
    {1,128,8,8,cir,0xff},   // 9 o'clock
    {254,128,8,8,cir,0xff},
    {128-90,128-90,8,8,cir,0xff},
    {128-90,128+90,8,8,cir,0xff},
    {128+90,128-90,8,8,cir,0xff},
    {128+90,128+90,8,8,cir,0xff},
    {128,128,128,0,pos,0x99},
    {128,128,0,128,pos,0x99},
    {255,255,0,0,cir,0x00},
  }; 
   uint8 masks[9] ={0,1,2,4,8,16,32,64,128};
   uint8 x,y,i;
   clear_buffer(MAIN_BUFFER);
  x=y=0;
  for(i=0;i<9;i++){
    test_pattern[0].seg_data.mask=masks[i]^0xff;
    clear_buffer(MAIN_BUFFER);
    compileSegments(test_pattern,MAIN_BUFFER,APPEND);

    while(!button_clicked){
    ss_x_offset = 0;
    ss_y_offset =0;
      display_buffer(MAIN_BUFFER);
    }
    button_clicked = 0;  // consume the button_click
      
  }
   
}
void align_screen(){
  seg_or_flag test_pattern[] = {
    {128,128,128,128,cir,0x0ff},
    {255,255,0,0,cir,0x00},
  }; 
   uint8 masks[9] ={0,1,2,4,8,16,32,64,128};
   uint8 x,y,i;
   clear_buffer(MAIN_BUFFER);
  x=y=0;
  compileSegments(test_pattern,MAIN_BUFFER,OVERWRITE);

  button_clicked = 0;  // consume the button_click
      
  while(!button_clicked){
    display_buffer(MAIN_BUFFER);
  }
   
}

// Routine for tracking decoder knob with feedback.  Used by several UI routines
int trackKnob(int initial_value,int min_value,int max_value, void display_proc(int value)){
    int result;
    int saved_knob = QuadDec_1_GetCounter();
    QuadDec_1_SetCounter(initial_value);
    
    while(!button_clicked){
        result = QuadDec_1_GetCounter();
        if (result < min_value || result > max_value){
            result = result < min_value ? min_value : max_value;
            QuadDec_1_SetCounter(result);
        }
        display_proc(result);  
    }
    button_clicked = 0;  // consume click
    QuadDec_1_SetCounter(saved_knob);  // restore knob setting
    return(result);
}
// UI feedback routine for "Set Locale"
void echo_locale(int x){
    char offset_buf[32];

    sprintf(offset_buf,"UTC Offset: %i", x);
    compileString(offset_buf,16,128,MAIN_BUFFER,1,0);
    display_buffer(MAIN_BUFFER);   

}
void set_locale(){
    int result = trackKnob(global_prefs.prefs_data.utc_offset,-12,14,echo_locale);
    global_prefs.prefs_data.utc_offset = result;
    flush_prefs();
}
// UI feedback routine for "Power Off"
void echo_power_off(int x){
    char offset_buf[32];

    sprintf(offset_buf,"Sleep in: %i minutes", x);
    compileString(offset_buf,16,128,MAIN_BUFFER,1,0);
    display_buffer(MAIN_BUFFER);   

}

extern time_t power_off_t;
void set_power_off(){
    RTC_1_TIME_DATE *psoc_now = RTC_1_ReadTime();
    time_t now= psoc_to_unix(psoc_now);
    int minutes_till_sleep = (power_off_t - now)/60;
    int result = trackKnob(minutes_till_sleep,0,180,echo_power_off);
    psoc_now = RTC_1_ReadTime();
    now= psoc_to_unix(psoc_now);
    power_off_t = now + result * 60;
    flush_prefs();
}

void show_sync(int s){
    char *strings[2] = {"Don't Sync","Sync"};
    clear_buffer(MAIN_BUFFER);
    compileString(strings[s],255,128,MAIN_BUFFER,1,OVERWRITE);
    display_buffer(MAIN_BUFFER);   

}
void set_sync(){
    int sync = global_prefs.prefs_data.sync_to_60Hz;
    global_prefs.prefs_data.sync_to_60Hz = trackKnob(sync,0,1,show_sync);
    flush_prefs();
}

void show_gps(int use_gps){
    char *strings[2] = {"Don't Use GPS","Use GPS"};
    clear_buffer(MAIN_BUFFER);
    compileString(strings[use_gps],255,128,MAIN_BUFFER,1,OVERWRITE);
    display_buffer(MAIN_BUFFER);   
}

void set_gps(){
    global_prefs.prefs_data.use_gps = trackKnob(global_prefs.prefs_data.use_gps,0,1,show_gps);
    flush_prefs();
}

void show_switch_interval(int i){
    char interval_buf[64];
    if(i != 0){
        sprintf(interval_buf,"Switch every %i sec", i);
    }
    else{
        sprintf(interval_buf,"Don't auto-switch");
    }
    compileString(interval_buf,16,128,MAIN_BUFFER,1,0);
    display_buffer(MAIN_BUFFER);   
}

void set_switch_interval(){
    int switch_interval = trackKnob(global_prefs.prefs_data.switch_interval,0,60,show_switch_interval);
    global_prefs.prefs_data.switch_interval = switch_interval;
    flush_prefs();
}

void dispatch_menu(int menu_number, int item_number){
  // save the decoder position, so that it makes sense upon returning:
  int prev_counter = QuadDec_1_GetCounter();
  switch (menu_number){
  case 0: /* main menu */
    switch (item_number){
    case 0:
      if(item_number == 0) set_the_time();
      break;
            
    case 1: 
      set_locale();
      break;
            
    case 2: 
      set_switch_interval();
      break;
            
    case 3: 
      char_test();
      break;
            
    case 4:
      align_screen2();
      break;
            
    case 5:
      set_gps();
      break;
    
    case 6:
      set_power_off();
      break;
    
    }                 
  }
  QuadDec_1_SetCounter(prev_counter);   // restore the knob position
}
