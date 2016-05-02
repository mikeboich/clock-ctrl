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

  void dispatch_menu(int menu_number, int item_number){
    switch (menu_number){
        case 0: /* main menu */
          set_time();
    }
}

    
