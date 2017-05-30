/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "gps.h"
#include <device.h>
#include <stdlib.h>
#include <stdio.h>
#include "UART_1.h"
#include "RTC_1.h"
#include "prefs.h"
#include "time.h"

#include "ds3231.h"

#define NEWLINE 0x0A
#define CR 0x0D

// handy constants:
uint8 days_in_month[2][12] = {{31,28,31,30,31,30,31,31,30,31,30,31},\
                            {31,29,31,30,31,30,31,31,30,31,30,31}};

char *field_n(uint8 n, char *sentence){
    char *c = sentence;
    while(n && *c){
        if(*c++ == ','){
            n--;
        }
    }
    if(n) return (0);  // we didn't get to field # n
    else return(c);  // c just advanced past the nth comma
}


void send_command(char *s){
    uint8 checksum = 0;
    char checksum_as_hex[8];
    UART_1_PutChar(*s++);  // send the $ character
    while(*s){
        UART_1_PutChar(*s);
        checksum = checksum ^ *s++;
    }
    UART_1_PutChar('*');
    sprintf(checksum_as_hex,"%02x",checksum);
    UART_1_PutChar(checksum_as_hex[0]);
    UART_1_PutChar(checksum_as_hex[1]);
    UART_1_PutChar(CR);
    UART_1_PutChar(NEWLINE);
}

void init_gps(){
    UART_1_Start();
    CyDelay(100);  // for luck
     //send_command("$PSRF100,1,38400,8,1,0");
     send_command("$PSRF103,00,00,00,01");

     send_command("$PSRF103,01,00,00,01");
     send_command("$PSRF103,02,00,00,01");
     send_command("$PSRF103,03,00,00,01");
    
     send_command("$PSRF103,04,00,01,01");
     send_command("$PSRF103,04,01,01,01");      // query this once to start the RTC

    send_command("$PSRF103,05,00,00,01");
     send_command("$PSRF103,06,00,00,01");
//send_command("$PSRF103,00,01,00,01");
    //hard_command();
    //RTC_1_WriteIntervalMask(RTC_1_INTERVAL_SEC_MASK);
    RTC_1_WriteIntervalMask(RTC_1_INTERVAL_SEC_MASK | RTC_1_INTERVAL_MIN_MASK);
    RTC_1_Start();
    RTC_1_Stop();
}
typedef enum {awaiting_dollar,awaiting_char, in_sentence} gps_parse_state;
extern int second_has_elapsed;
int sentence_avail;
char sentence[256] = "Hello World";

// convert two successive decimal digits to an int:
int a_to_int(char *s){
    return 10*(s[0] - 0x30) + (s[1] - 0x30);
}

// Sets the RTC to equal the incoming GPS Time, EXCEPT, if the incoming time is just one second
// greater than the current RTC time, we'll optimistically assume that the onePPS interrupt will arrive 
// in the next second, and set the time for us.
// because I'm lazy, I only check this when decrementing the incoming second count doesn't cause an underflow
// 
void set_rtc_to_gps(){
    extern int pps_available;
    static int seed = 0;
    static int time_set=0;

    //RTC_1_TIME_DATE *rtc_time = RTC_1_ReadTime();
    time_t rtc_time = get_DS3231_time();
    //RTC_1_TIME_DATE gps_time;
    
    struct tm tm_gps,tm_rtc;
    tm_rtc = *gmtime(&rtc_time);

    tm_gps.tm_hour = a_to_int(field_n(1,sentence));
    tm_gps.tm_min = a_to_int(field_n(1,sentence)+2);
    tm_gps.tm_sec = a_to_int(field_n(1,sentence)+4); 
    //offset_time(&gps_time,global_prefs.prefs_data.utc_offset);  // so we can compare to RTC, which is already offset
   
//    if(time_set && tm_gps.tm_sec!=0){
//        if(tm_gps.tm_sec-1 == tm_rtc.tm_sec && tm_gps.tm_min==tm_rtc.tm_min && tm_gps.tm_hour==tm_rtc.tm_hour)
//          return;  // do nothing in this case
//    }
    // if we reach this point, we need to set the time:
    tm_gps.tm_mday = a_to_int(field_n(9,sentence));
    tm_gps.tm_mon = a_to_int(field_n(9,sentence+2))-1;  //  map 1..12 to 0..11
    tm_gps.tm_year = 100 + a_to_int(field_n(9,sentence+4));
    tm_gps.tm_isdst = 0;
    time_t gps_time = mktime(&tm_gps);
    setDS3231(gps_time);
    //offset_time(rtc_time,global_prefs.prefs_data.utc_offset);
    
    // INTERIM code to set DS3231 to GPS
    
    // create a seed for the random number generator based on time and date:
    if(seed==0){
        seed = tm_gps.tm_sec+60*tm_gps.tm_min+3600*tm_gps.tm_hour+86400*tm_gps.tm_yday;
        srand(seed);
    }
    time_set=1;
    
}

void consume_char(char c){
    static gps_parse_state state = awaiting_char;
    char expected_char[] = "$GPRMC";
    //char expected_char[] = "$GPGGA";
    
    static uint8 index=0;
    static uint8 buf_index=0;
    
    switch(state){
        case awaiting_dollar:
          if(c=='$'){
            state = awaiting_char;
            index=1;
            sentence[buf_index++]=c;
            }
          break;
            
        case  awaiting_char:
            if(c == expected_char[index]){
              index += 1;
              sentence[buf_index++] = c;
              if(expected_char[index]==0){  // we've found all the expected_char characters
                state = in_sentence;
              }      
            }
            else {  // we have to start over
                state = awaiting_dollar;
                index=0;
                buf_index=0;
            }
           break;
            
            case in_sentence:
              sentence[buf_index++] = c;
              if(c==NEWLINE){
                sentence_avail=1;
                sentence[buf_index++] = 0;  //terminate the string
                index=0;
                buf_index=0;
                state = awaiting_dollar;
                
               // second_has_elapsed = 1;
               set_rtc_to_gps();
            }
            break;
    }
}
void offset_time(RTC_1_TIME_DATE *t, int hours){
    int h = t->Hour + hours;
    if(h>23){
     h -= 24;
     increment_date(t,1);
    }
    else{
     if(h < 0){
       h += 24;
       increment_date(t,-1);
      }
    }
    t->Hour = h;
}

int is_leap_year(int y){
 if(4*(y/4) != y) return 0;
 // divisible by 4.  if not visible by 100, it's a leap year:
 if(!(100*(y/100)==y)) return(1);

// divisible by 4 and by 100.  Leap year if divisible by 400:
 if(y == 400*(y/400)) return 1;
 else return(0);
}
void increment_date(RTC_1_TIME_DATE *t,int incr){
    int d,m,y,dayOfWeek;
    int year_type = is_leap_year(y);  // 0 for regular, 1 for leap
    d = t->DayOfMonth;
    m = t->Month;
    y = t->Year;
    dayOfWeek = t->DayOfWeek;
    
    d += incr;  // increment or decrement the date incr should be +1 or -1 only
    if(d > days_in_month[year_type][m-1]){
        m +=1;
        d=1;
        if(m>12){  // we went into the new year
            m=1;
            y+=1;
        }
    }
    
    if(d==0){
        m-=1;  // go to prev month
        if(m<1){  // if month goes to zero, wrap to December 31 of prior year:
         m+=12;
         y-=1;
         d=31;  // i.e. 12/31/prev-year
        }
       else d = days_in_month[year_type][m-1];  //wrap to last day of previous month
    }
    t->DayOfMonth=d;
    t->Month=m;
    t->Year = y;
    
    
}
// testing to see if fields are present in tm struct:
void test(){
    struct tm tm_test={.tm_mon=11, .tm_isdst=1};
}
/* [] END OF FILE */
