/*  menus.h

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
#include "font.h"

volatile int button_clicked;

// Some useful strings:
extern char *day_names[7];
char *month_names[12];
extern int8 gmt_offset;

typedef struct {char *items[8]; int n_items; int highlighted_item_index; uint8 menu_number;} menu;

extern menu main_menu;
extern menu *active_menu;

// utility functions:
void sync_to_60Hz();
void wait_for_twist();

// display a menu:
void render_menu(menu the_menu);

void compile_menu(menu *the_menu, int which_buffer);

void dispatch_menu(int menu_number, int item_number);