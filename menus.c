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
#include <stdlib.h>
#include <stdio.h>

extern int cycle_count;
int button_clicked;

// Some useful strings:
char *day_names[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
char *month_names[12] = {"Jan", "Feb", "Mar", "April","May","June","July","Aug","Sep","Oct","Nov","Dec"};

menu main_menu = {.items = {"Set Time/Date","Set Locale", "Align Screen","Cancel"},
          .n_items = 4,
		  .highlighted_item_index = -1,
          .menu_number = 0};

menu *current_menu = &main_menu;



void display_menu(menu the_menu){
  compile_menu(&the_menu,MENU_BUFFER);
  //active_menu = &the_menu;
  
}

void compile_menu(menu *the_menu, int which_buffer){
 int item_index;
 int y;
 char item_str[80] = "";
 
 y = 128+(the_menu->n_items/2)*40;
 clear_buffer(MENU_BUFFER);
 for(item_index=0;item_index<the_menu->n_items;item_index++){
    item_str[0] = 0;
    if(item_index==the_menu->highlighted_item_index) strcat(item_str,">");
    strcat(item_str,the_menu->items[item_index]);
    compileString(item_str,255,y,which_buffer,1,APPEND);
    y -= 40;
  }
}

void set_time(){
  enum {h,m,s,mo,d,y} selected;
  RTC_1_TIME_DATE *the_time,our_time;
  the_time = RTC_1_ReadTime();
  our_time = *the_time;
}

void pack_time(RTC_1_TIME_DATE *t, int a[]){
    a[0] = t->Month;
    a[1] = t->DayOfMonth;
    a[2] = t->Year;
    
    a[3] = t->Hour;
    a[4] = t->Min;
    a[5] = t->Sec;
    
    a[6] = t->DayOfWeek;
}

void unpack_time(RTC_1_TIME_DATE *t, int a[]){
    t->Month = a[0];
    t->DayOfMonth = a[1];
    t->Year = a[2];

    t->Hour = a[3];
    t->Min = a[4];
    t->Sec = a[5];

    t->DayOfWeek = a[6];
}

    struct {RTC_1_TIME_DATE the_time;
       int selected_field;
} time_set_state;

int blink_time(){
    return ((cycle_count % 31250) > 15625);
}
void compile_time_screen(int a[],int selected_field){
    int field;
    int x_pos[7] = {128-40,128,128+40,128-40,128,128+40,128-64};
    int y_pos[7] = {200,200,200,110,110,110};
    seg_or_flag segment;
    char num_buf[8];
    char format_str[7][8] = {"%02i","%02i","%i","%02i","%02i","%02i","%s"};
    
    clear_buffer(ANALOG_BUFFER);
    for(field=0; field<6; field++){
        num_buf[0] = 0;     // make string null string
        sprintf(num_buf, format_str[field],a[field]);
        if(selected_field!=field || blink_time())
          compileString(num_buf,x_pos[field],y_pos[field],ANALOG_BUFFER,1,APPEND);
    }
    if(selected_field!=field || blink_time())
      compileString(day_names[a[6]],x_pos[6],y_pos[6],ANALOG_BUFFER,1,APPEND);
}
int menu_button_state = 1;

void display_buffer(uint8 which_buffer);



void set_the_time(){
    int time_params[7];
    RTC_1_TIME_DATE *clock_time;
    clock_time = RTC_1_ReadTime();
    int selected_field=0;
    int field_select_mode=0;
    int prev_counter = QuadDec_1_GetCounter();
    int field_min[7] = {0,0,2016,0,0,0,0};
    int field_max[7] = {12,31,2500,23,59,59,6};
    
    pack_time(clock_time,time_params);
    
    for(;;){
      compile_time_screen(time_params,selected_field);
      display_buffer(ANALOG_BUFFER);
      if(field_select_mode){
        selected_field = QuadDec_1_GetCounter() % 7;
    }
      else {
        int new_counter = QuadDec_1_GetCounter();
        if(prev_counter != new_counter){
         time_params[selected_field] += (new_counter-prev_counter); 
         if(time_params[selected_field] > field_max[selected_field]){
           time_params[selected_field] = time_params[selected_field] % (field_max[selected_field] + 1);   
        }
         prev_counter = new_counter;
        }
      }
    if(button_clicked){
        button_clicked=0;  // consume the click..
        if(field_select_mode){
            field_select_mode=0;
            prev_counter = QuadDec_1_GetCounter();
        }
        else {
            //field_select_mode = 1;
            selected_field = (selected_field + 1) % 7;     
        }
    }
    }
    
    
}

  void dispatch_menu(int menu_number, int item_number){
    switch (menu_number){
        case 0: /* main menu */
          if(item_number == 0) set_the_time();
    }
}