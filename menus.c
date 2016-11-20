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
#include "menus.h"
#include "draw.h"
#include "prefs.h"
#include <stdlib.h>
#include <stdio.h>

extern int cycle_count;
extern int verbose_mode;
//int button_clicked;

// Some useful strings:
char *day_names[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
char *month_names[12] = {"Jan", "Feb", "Mar", "April","May","June","July","Aug","Sep","Oct","Nov","Dec"};

menu main_menu = {.items = {"Set Time/Date","Set Locale","Autoswitch","Character Test","Align Screen","Cancel"},
		  .n_items = 5,
		  .highlighted_item_index = -1,
		  .menu_number = 0};

menu *current_menu = &main_menu;




void display_menu(menu the_menu){
  compile_menu(&the_menu,MAIN_BUFFER);
  //active_menu = &the_menu;
  
}

void compile_menu(menu *the_menu, int which_buffer){
  int item_index;
  int y;
  char item_str[80] = "";
 
  y = 128+(the_menu->n_items/2)*40;
  clear_buffer(MAIN_BUFFER);
  for(item_index=0;item_index<the_menu->n_items;item_index++){
    item_str[0] = 0;
    if(item_index==the_menu->highlighted_item_index) strcat(item_str,"->");
    strcat(item_str,the_menu->items[item_index]);
    compileString(item_str,255,y,which_buffer,1,APPEND);
    y -= 40;
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
  RTC_1_TIME_DATE *clock_time;
  clock_time = RTC_1_ReadTime();
  int selected_field=0;
  int prev_counter = QuadDec_1_GetCounter();
  int field_min[7] = {0,0,2016,0,0,0,0};
  int field_max[7] = {12,31,2500,23,59,59,6};
  uint8 done=0;
    
  unpack_time(clock_time,time_params);
    
  while(!done){
    compile_time_screen(time_params,selected_field);
    display_buffer(MAIN_BUFFER);
    int new_counter = QuadDec_1_GetCounter();
    if(prev_counter != new_counter){
      time_params[selected_field] += (new_counter-prev_counter); 
      if(time_params[selected_field] > field_max[selected_field]){
	time_params[selected_field] = time_params[selected_field] % (field_max[selected_field] + 1);   
      }
      prev_counter = new_counter;
    }
    if(button_clicked){
      button_clicked=0;  // consume the click..
      if(selected_field==6){
	RTC_1_TIME_DATE *the_time = RTC_1_ReadTime();
	RTC_1_DisableInt();
	pack_time(the_time,time_params);
	RTC_1_WriteTime(the_time);
	RTC_1_EnableInt();
	done=1;
      }
      else {
	selected_field = (selected_field + 1) % 7; 
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
    {128,128,128,128,cir,0x0ff},
    {128,192,8,8,cir,0xff},
    {128,64,8,8,cir,0xff},
    {64,128,8,8,cir,0xff},
    {192,128,8,8,cir,0xff},
    {128-45,128-45,8,8,cir,0xff},
    {128-45,128+45,8,8,cir,0xff},
    {128+45,128-45,8,8,cir,0xff},
    {128+45,128+45,8,8,cir,0xff},
    {128,128,64,0,pos,0x99},
    {128,128,0,64,pos,0x99},
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
      display_buffer(MAIN_BUFFER);
    }
    button_clicked = 0;  // consume the button_click
      
  }
   
}
void align_screen(){
  seg_or_flag test_pattern[] = {
    {128,128,128,128,cir,0x0ff},
    {128,192,8,8,cir,0xff},
    {128,64,8,8,cir,0xff},
    {64,128,8,8,cir,0xff},
    {192,128,8,8,cir,0xff},
    {128-45,128-45,8,8,cir,0xff},
    {128-45,128+45,8,8,cir,0xff},
    {128+45,128-45,8,8,cir,0xff},
    {128+45,128+45,8,8,cir,0xff},
    {128,128,64,0,pos,0x99},
    {128,128,0,64,pos,0x99},
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
      display_buffer(MAIN_BUFFER);
    }
    button_clicked = 0;  // consume the button_click
      
  }
   
}

void set_locale(){
    int utc_offset = global_prefs.prefs_data.utc_offset;
    int saved_decoder = QuadDec_1_GetCounter();
    char offset_buf[32];
    int new_offset;
    
    while(! button_clicked){
        new_offset = (QuadDec_1_GetCounter() - saved_decoder)+utc_offset;
        if(new_offset>23) new_offset -= 24;
        if(new_offset<-23) new_offset += 24;
        
        sprintf(offset_buf,"UTC Offset: %i", new_offset);
        compileString(offset_buf,16,128,MAIN_BUFFER,1,0);
        display_buffer(MAIN_BUFFER);   
    }
    button_clicked = 0;
    global_prefs.prefs_data.utc_offset = new_offset;
    flush_prefs();
    QuadDec_1_SetCounter(saved_decoder);
}
void set_switch_interval(){
    int switch_interval = global_prefs.prefs_data.switch_interval;
    int saved_decoder = QuadDec_1_GetCounter();
    char interval_buf[32];
    int new_interval;
    
    while(! button_clicked){
        new_interval = (QuadDec_1_GetCounter() - saved_decoder)+switch_interval;
        if(new_interval>60) new_interval -= 60;
        if(new_interval<0) new_interval +=60;
        
        if(new_interval != 0){
            sprintf(interval_buf,"Switch every %i sec", new_interval);
        }
        else{
            sprintf(interval_buf,"Don't auto-switch");
        }
        compileString(interval_buf,16,128,MAIN_BUFFER,1,0);
        display_buffer(MAIN_BUFFER);   
    }
    button_clicked = 0;
    global_prefs.prefs_data.switch_interval = new_interval;
    flush_prefs();
    QuadDec_1_SetCounter(saved_decoder);    
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
      align_screen();
      break;
            
    case 5: 
      break;
    }                 
  }
  QuadDec_1_SetCounter(prev_counter);   // restore the knob position
}
