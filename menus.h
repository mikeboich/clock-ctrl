/*  menu.h

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
#include <stdio.h>
#include <stdlib.h>
#include "font.h"

typedef struct {char items[], int highlighed_item_index} menu;

menu main_menu = {.items = {"Set Time/Date","Set Locale", "Align Screen"},
		  .highlighted_item_index = -1};

// present a menu and return the index of the selected item (or -1 for cancel, -2 for no selection yet):
int track_menu(menu the_menu);

// display a menu:
void display_menu(menu the_menu);

void compile_menu(menu the_menu, int which_buffer);

