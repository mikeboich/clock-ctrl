/* ========================================
 *
    Copyright (C) 2016 Michael Boich

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    Routines for the Maxim DS3231 real time clock chip
 *
 * ========================================
*/
#include "ds3231.h"


// time conversion routines.  If I had a "utils" module, I'd have put these in there.
void unix_to_psoc(time_t the_time, RTC_1_TIME_DATE *result){
    struct tm unix_time;
    
    unix_time = *gmtime(&the_time);
    result->Month = unix_time.tm_mon+1;
    result->DayOfMonth = unix_time.tm_mday;
    result->Year = unix_time.tm_year+1900;
    
    result->Hour = unix_time.tm_hour;
    result->Min = unix_time.tm_min;
    result->Sec = unix_time.tm_sec;
    result->DayOfWeek = unix_time.tm_wday+1;
}

time_t psoc_to_unix(RTC_1_TIME_DATE *rtc_time){
    struct tm psoc_tm;
    psoc_tm.tm_hour = rtc_time->Hour;
    psoc_tm.tm_min = rtc_time->Min;
    psoc_tm.tm_sec = rtc_time->Sec;
    psoc_tm.tm_mday = rtc_time->DayOfMonth;
    psoc_tm.tm_mon = rtc_time->Month-1;
    psoc_tm.tm_year = rtc_time->Year-1900;
    psoc_tm.tm_isdst = 0;
    return(mktime(&psoc_tm));
    
}

uint8_t bcd_to_bin(uint8_t in_byte){
    return(((in_byte & 0b11110000) >> 4) * 10 + (in_byte & 0b00001111));
}

uint8_t bin_to_BCD(uint8_t x){
    uint8 hi_nybble = (x / 10);
    uint lo_nybble = x % 10;
    return((uint8_t) (hi_nybble << 4) | lo_nybble);
}

#define DS3231_Addr 0x68
time_t get_DS3231_time(){
    time_t t;
    uint8_t inbuf[32],outbuf[32];
    struct tm the_time;
    uint8_t err;
    
    // Tell the DS3231 we want to read from byte 0:
    outbuf[0] = 0;      // sending just this 0 byte
    I2C_1_MasterWriteBuf(DS3231_Addr,outbuf,1,I2C_1_MODE_COMPLETE_XFER);
    CyDelay(1);
    
    err = I2C_1_MasterReadBuf(DS3231_Addr,inbuf,7,I2C_1_MODE_COMPLETE_XFER);
    
    CyDelay(1);
    //while(I2C_1_MasterStatus()== I2C_1_MSTAT_XFER_INP){
    //};  // spin until transfer complete
 
    
    the_time.tm_sec = bcd_to_bin(inbuf[0]);
    the_time.tm_min = bcd_to_bin(inbuf[1]);
    the_time.tm_hour  = (((inbuf[2] & 0b00100000) >> 5)*20) + (((inbuf[2] & 0b00010000) >> 4)*10) +( inbuf[2] & 0b00001111);
    
    the_time.tm_mday = ((inbuf[4] & 0b00110000) >> 4) *10 + (inbuf[4] & 0b00001111);
    the_time.tm_mon = ((inbuf[5] & 0b00010000) >> 4) * 10 + (inbuf[5] & 0b00001111) - 1;  // months are 0..11 vs 1..12
    the_time.tm_year = bcd_to_bin(inbuf[6])+ 100;  // DS3231 keeps only year mod 100. assume 2000-2100 range
    the_time.tm_isdst = 0;
       
    return(mktime(&the_time));
}


void setDS3231(time_t time_now){
    struct tm time_components;
    uint8_t out_buf[16];
    uint8_t test_buf[8] = {0,33,34,35,36,37,38,39};
    
    time_components = *gmtime(&time_now);
   
    // lay the components out in DS3231 format:
    out_buf[0] = 0; // internal pointer 
    out_buf[1] = bin_to_BCD(time_components.tm_sec);
    out_buf[2] = bin_to_BCD(time_components.tm_min);
    out_buf[3] = bin_to_BCD(time_components.tm_hour);   
    
    out_buf[4] = bin_to_BCD(time_components.tm_wday + 1);  // map 0..6 to 1..7
    out_buf[5] = bin_to_BCD(time_components.tm_mday);
    out_buf[6] = bin_to_BCD(time_components.tm_mon+1);     //map 0..11 to 1..12
    out_buf[7] = bin_to_BCD(time_components.tm_year % 100);   // year is modulo 100  Y2K madness!
    out_buf[8] = 0;     // dummy sentinel for debug
    
    I2C_1_MasterWriteBuf(DS3231_Addr,out_buf,8,I2C_1_MODE_COMPLETE_XFER);
    CyDelay(1);
}

void write_DS3231_status_reg(uint8_t bits){
    uint8_t out_buf[2] = {0x0E,0};  // 0x0E is address of status reg
    
    out_buf[1] = bits;
     I2C_1_MasterWriteBuf(DS3231_Addr,out_buf,2,I2C_1_MODE_COMPLETE_XFER);
    CyDelay(1);
   
}
/* [] END OF FILE */
