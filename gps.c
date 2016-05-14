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
#include <device.h>
#include <stdlib.h>
#include <stdio.h>
#include "UART_1.h"
#include "RTC_1.h"

#define NEWLINE 0x0A
#define CR 0x0D

uint8 time_offset = -7;

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
     //send_command("$PSRF103,04,01,01,01");      // query this once to start the RTC

    send_command("$PSRF103,05,00,00,01");
     send_command("$PSRF103,06,00,00,01");
//send_command("$PSRF103,00,01,00,01");
    //hard_command();
}
typedef enum {awaiting_dollar,awaiting_char, in_sentence} gps_parse_state;
extern int second_has_elapsed;
int sentence_avail;
char sentence[256] = "Hello World";

// convert two successive decimal digits to an int:
uint8 a_to_uint8(char *s){
    return 10*(s[0] - 0x30) + (s[1] - 0x30);
}
void set_rtc_to_gps(){
    RTC_1_TIME_DATE *t = RTC_1_ReadTime();
    //char *c = field_n(&sentence[9]);
    t->Hour = a_to_uint8(field_n(1,sentence));
    t->Min = a_to_uint8(field_n(1,sentence)+2);
    t->Sec = a_to_uint8(field_n(1,sentence)+4);   
    
    t->DayOfMonth = a_to_uint8(field_n(9,sentence));
    t->Month = a_to_uint8(field_n(9,sentence)+2);
    t->Year = 2000+ a_to_uint8(field_n(9,sentence)+4);
    RTC_1_Start();
    RTC_1_Stop();
   
}

void consume_char(char c){
    static gps_parse_state state = awaiting_char;
    //char expected_char[] = "$GPRMC";
    char expected_char[] = "$GPRMC";
    
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
