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
#include <stdlib.h>
#include <stdio.h>
#include "UART_1.h"
#include "RTC_1.h"


void init_gps(){
    UART_1_Start();
}

char *get_utc_time(){
    static char result[256] = "";
    static uint8 index=0;
    char ch=0;
    
    int avail = UART_1_GetTxBufferSize();
    while(avail-- && index < 255 && ch != (char) 13){
        ch = UART_1_GetChar();
        result[index++] = ch;
    }
    
    index=0;
    ch = ' ';
    return result;
}


typedef enum {awaiting_dollar,awaiting_char, in_sentence} gps_parse_state;
extern int second_has_elapsed;
#define NEWLINE 0x0A
int sentence_avail;
char sentence[256] = "Hello World";

void set_rtc_to_gps(){
    RTC_1_TIME_DATE *t = RTC_1_ReadTime();
    char *time_data = &sentence[7];
    char hour[5];
    char minute[3];
    char seconds[3];
    int i,h,m,s;
    
    for(i=0;i<2;i++) hour[i] = *time_data++;
    hour[2] = 0;
    for(i=0;i<2;i++) minute[i] = *time_data++;
    minute[2] = 0;
    for(i=0;i<2;i++) seconds[i] = *time_data++;
    seconds[2] = 0;
    
    h = atoi(hour);
    m = atoi(minute);
    s = atoi(seconds);
    
    t->Hour = h;
    t->Min = m;
    t->Sec = s;
    
}

void consume_char(char c){
    static gps_parse_state state = awaiting_char;
    char expected_char[] = "$GPGGA";
    
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
                index=0;
                buf_index=0;
                state = awaiting_dollar;
                
                second_has_elapsed = 1;
                set_rtc_to_gps();
            }
            break;
    }
}
/* [] END OF FILE */
