//
//  ViewingLocation.h
//  Lunar Foundation
//
//  Created by Michael Boich on 10/1/09.
//  Copyright 2009 Self. All rights reserved.
//

#include "time.h"
#include "math.h"

struct location {
  float latitude, longitude;
  time_t viewing_date;
  int gmt_offset;
};



  void init_location(struct location *l);
// time_t calcSunOrMoonRiseForDate(time_t the_date, int riseOrSet, int sunOrMoon);

