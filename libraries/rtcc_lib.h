/*
 * File:   rtcc_lib.h
 * Author: kirstenhipolito_EEE
 *
 * Created on May 19, 2019 9:00 PM
 */

/*
 * A library of RTCC functions.
 * Made by: Kirsten Rae C. Hipolito
 * 2019 May
 *
 */

#include "xc.h"
#include "stdio.h"
#include "string.h"
#define FCY 4000000UL
#include "libpic30.h"
#include "time.h"

int format_year = 0;    //0 = 2019; 1 = 19
int format_month = 0;     //0 = string (Jan); 1 = number (1)
int format_time = 0;    //0 = 24H (20:00); 1 = 12H (08:00 PM)
int monthdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

char line1[16] = {0};
char line2[16] = {0};

void rtcc_initialize(void);
//initialize the RTCC module; initializes it to the day I wrote this!

void rtcc_settime(const struct tm time);
//set the RTCC module's time

void rtcc_gettime(struct tm *time);
//get the time from the RTCC module, which will be put in the input struct tm pointer time

void rtcc_printdate(struct tm *inputtime);
//print the date indicated in the input struct tm pointer inputtime

void rtcc_printtime(struct tm *inputtime, int withsec);
//print the time indicated in the input struct tm pointer inputtime

char *getmonth(int month);
//get month string from its int value

char *getweekday(int weekday);
//get weekday string from its int value

//other processing functions for rtcc
int hextodec(const int input_num);
int dectohex(const int input_num);

void rtcc_initialize(void){   
    //configure RTCC
    RCFGCAL = 0x00;
    __builtin_write_RTCWEN();   //enable writing to RTCC
    RCFGCALbits.RTCEN = 0;      //disable RTCC
    RCFGCALbits.RTCPTR = 0b11;  //point RTCVAL to (reserved)/YEAR
    RTCVAL = 0x2019;   //write 19 as year
    RTCVAL = 0x0519; //write 05 as month and 19 as day
    RTCVAL = 0x0020; //write day of week as 0 and hour as 20
    RTCVAL = 0x4059; //write 40 as minute and 59 as second
    RCFGCALbits.RTCEN = 1;      //enable RTCC
    __delay_us(50);
    RCFGCALbits.RTCWREN = 0;    //disable writing to RTCC
}

void rtcc_settime(const struct tm time){
    __builtin_write_OSCCONL( 0x02 );
    __builtin_write_RTCWEN();   //enable writing to RTCC
    RCFGCALbits.RTCEN = 0;      //disable RTCC
    RCFGCALbits.RTCPTR = 0b11;  //point RTCVAL to (reserved)/YEAR
    
    RTCVAL = hextodec(time.tm_year);  //write year
    RTCVAL = (hextodec(time.tm_mon) << 8) | (hextodec(time.tm_mday)); //write month and day
    RTCVAL = (hextodec(time.tm_wday) << 8) | (hextodec(time.tm_hour)); //write day of week and hour
    RTCVAL = (hextodec(time.tm_min) << 8) | (hextodec(time.tm_sec)); //write minute and second
    
    RCFGCALbits.RTCEN = 1;      //enable RTCC
    __delay_us(50);
    RCFGCALbits.RTCWREN = 0;    //disable writing to RTCC
}

void rtcc_gettime(struct tm *time){
    static int time_holder[4] = {0,0,0,0};
    static int i = 0;
    RCFGCALbits.RTCPTR = 0b11;
    
    for (i = 0; i < 4; i++){
        time_holder[i] = RTCVAL;
    }
    
    time->tm_year = dectohex(time_holder[0]);
    time->tm_mon = dectohex(time_holder[1] >> 8);
    time->tm_mday = dectohex(time_holder[1] & 0x00FF);
    time->tm_wday = dectohex(time_holder[2] >> 8);
    time->tm_hour = dectohex(time_holder[2] & 0x00FF);
    time->tm_min = dectohex(time_holder[3] >> 8);
    time->tm_sec = dectohex(time_holder[3] & 0x00FF);
}

void rtcc_printdate(struct tm *inputtime){
    if (inputtime == NULL){
        struct tm timenow;
        rtcc_gettime(&timenow);
        inputtime = &timenow;
    }
    
    strcpy(line1, "");

    if (format_year == 0) {
        if (format_month == 0){
            sprintf(line1, "%02d %s 20%02d, %s", inputtime->tm_mday, getmonth(inputtime->tm_mon), inputtime->tm_year, getweekday(inputtime->tm_wday));
        } else if (format_month == 1) {
            sprintf(line1, "%02d %02d  20%02d, %s", inputtime->tm_mday, inputtime->tm_mon, inputtime->tm_year, getweekday(inputtime->tm_wday));
        }
    } else if (format_year == 1) {
        if (format_month == 0){
            sprintf(line1, "%02d %s %02d,   %s", inputtime->tm_mday, getmonth(inputtime->tm_mon), inputtime->tm_year, getweekday(inputtime->tm_wday));
        } else if (format_month == 1) {
            sprintf(line1, "%02d %02d  %02d,   %s", inputtime->tm_mday, inputtime->tm_mon, inputtime->tm_year, getweekday(inputtime->tm_wday));
        }
    }
    
    lcd_printstring(line1);
    
}

void rtcc_printtime(struct tm *inputtime, int withsec){
    if (inputtime == NULL){
        struct tm timenow;
        rtcc_gettime(&timenow);
        inputtime = &timenow;
    }
    
    strcpy(line1, "");
    if (withsec == 1){
        if (format_time == 0){
            sprintf(line1, "  %02d:%02d:%02d       ", inputtime->tm_hour, inputtime->tm_min, inputtime->tm_sec);
        }
        else if (format_time == 1){
            int diff = inputtime->tm_hour - 12;

            if (diff < 0){
                sprintf(line1, "  %02d:%02d:%02d AM     ", inputtime->tm_hour, inputtime->tm_min, inputtime->tm_sec);
            } else if (diff == 0){
                sprintf(line1, "  %02d:%02d:%02d PM     ", inputtime->tm_hour, inputtime->tm_min, inputtime->tm_sec);
            }else if (diff > 0){
                sprintf(line1, "  %02d:%02d:%02d PM     ", diff, inputtime->tm_min, inputtime->tm_sec);
            }   
        }
    } else if (withsec == 0){
        if (format_time == 0){
            sprintf(line1, "  %02d:%02d       ", inputtime->tm_hour, inputtime->tm_min);
        }
        else if (format_time == 1){
            int diff = inputtime->tm_hour - 12;

            if (diff < 0){
                sprintf(line1, "  %02d:%02d AM     ", inputtime->tm_hour, inputtime->tm_min);
            } else if (diff == 0){
                sprintf(line1, "  %02d:%02d PM     ", inputtime->tm_hour, inputtime->tm_min);
            }else if (diff > 0){
                sprintf(line1, "  %02d:%02d PM     ", diff, inputtime->tm_min);
            }   
        }
    }
    
    
    lcd_printstring(line1);
    
}

void rtcc_setalarm(struct tm timealarm, int mask){   
    ALCFGRPTbits.ALRMEN = 0;        // disable alarm

    ALCFGRPTbits.ALRMPTR = 2;       // point ALRMVAL to start
    ALRMVAL = (hextodec(timealarm.tm_mon) << 8) | (hextodec(timealarm.tm_mday)); //write month and day
    ALRMVAL = (hextodec(timealarm.tm_wday) << 8) | (hextodec(timealarm.tm_hour)); //write day of week and hour
    ALRMVAL = (hextodec(timealarm.tm_min) << 8) | (hextodec(timealarm.tm_sec)); //write minute and second

    // set the repeat counter
    ALCFGRPTbits.ARPT = 255;          // configure repeat
    ALCFGRPTbits.CHIME = 1;         // configure chime

    // set the alarm mask 
    ALCFGRPTbits.AMASK = mask;
    
    ALCFGRPTbits.ALRMEN = 1;        // enable alarm
    IFS3bits.RTCIF = 0;             // clear alarm interrupt flag
    IEC3bits.RTCIE = 1;             // enable alarm interrupt
}

int hextodec(const int input_num){
    static int tens = 0, ones = 0;
    
    tens = input_num/10;
    ones = input_num - (tens*10);
    
    return (tens*16) + ones;
}

int dectohex(const int input_num){
    static int tens = 0, ones = 0;
    
    ones = input_num % 16;
    tens = (input_num - ones) / 16;
    
    return (tens*10) + ones;
}

char *getmonth(int month){
    if (month == 1) return "Jan";
    else if (month == 2) return "Feb";
    else if (month == 3) return "Mar";
    else if (month == 4) return "Apr";
    else if (month == 5) return "May";
    else if (month == 6) return "Jun";
    else if (month == 7) return "Jul";
    else if (month == 8) return "Aug";
    else if (month == 9) return "Sep";
    else if (month == 10) return "Oct";
    else if (month == 11) return "Nov";
    else if (month == 12) return "Dec";
    else if (month == -1) return "---";
    
    return NULL;
    
}

char *getweekday(int weekday){
    if (weekday == 0) return "Sun";
    else if (weekday == 1) return "Mon";
    else if (weekday == 2) return "Tue";
    else if (weekday == 3) return "Wed";
    else if (weekday == 4) return "Thu";
    else if (weekday == 5) return "Fri";
    else if (weekday == 6) return "Sat";
    else if (weekday == -1) return "---";
    
    return NULL;
}
