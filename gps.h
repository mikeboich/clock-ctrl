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
 * GPS and time-keeping routinesL
*/
#include <RTC_1.h>
extern int sentence_avail;
extern char sentence[256];

void init_gps();
void send_command(char *s); // send a command with checksum and crlf

// state machine that consumes characters and constructs sentence(s):
void consume_char(char c);

void offset_time(RTC_1_TIME_DATE *t, int hours);
void increment_date(RTC_1_TIME_DATE *t, int incr);
void decrement_time(RTC_1_TIME_DATE *t, int hours);
void decrement_date(RTC_1_TIME_DATE *t);

/* [] END OF FILE */
