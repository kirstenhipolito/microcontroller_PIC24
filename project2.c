/*
 * File:   main.c
 * Author: kirstenhipolito_EEE
 *
 * Created on February 26, 2019, 5:50 PM
 */

#include "xc.h"
#define FCY 4000000UL
#include "libpic30.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "math.h"
#include "lcd_lib.h"
#include "keypad_lib.h"
#include "adc_lib.h"
#include "rtcc_lib.h"

// CONFIG4
#pragma config RTCOSC = LPRC            // RTCC Reference Oscillator  Select (RTCC uses Secondary Oscillator (SOSC))

_CONFIG1 (FWDTEN_OFF & JTAGEN_OFF)
_CONFIG2 (POSCMOD_NONE & OSCIOFNC_ON & FCKSM_CSDCMD & FNOSC_FRCPLL & PLL96MHZ_OFF & PLLDIV_NODIV)
_CONFIG3 (SOSCSEL_IO)
        
        
#define DEBOUNCEMAX 100
        
void __attribute__ ((interrupt)) _CNInterrupt(void); 
void config_OC2(void);


int screenSelect = 0;
int changeThis = 0; 
    // which part of date/time to change
    // in CTF: 0 = Month Day, 1 = Month, 2 = Year, 3 = Weekday, 4 = Hour, 5 = Minute, 6 = OK
    // in DTF: 0 = Month Format, 1 = Year Format, 2 = Time Format
    //in SetLight and SetCurtain, 0 = open hour, 1 = open min, 2 = close hour, 3 = close min
struct tm timealarm = {0};  //timeholder for alarm
struct tm timelighton = {0}, timelightoff = {0};
struct tm timecurtainopen = {0}, timecurtainclose = {0};
int alarmsound = 0; //what sound the alarm will take
char alarmreminder[16] = {0};   //reminder for set alarm;


int main(void) {
    lcd_initialize();
    keypad_initialize();
    adc_initialize();
    rtcc_initialize();
    
    char line1[16];
    char line2[16];

    //configure  input change notification interrupt for pushbutton
    CNEN2bits.CN30IE = 1;       //enable CN for RA2 (pushbutton)
    IEC1bits.CNIE = 1;          //enable CN interrupt
    IPC4bits.CNIP = 0x5;        //set CN interrupt priority
    IFS1bits.CNIF = 0;          //clear CN interrupt flag
    
    //configure ADC
    IEC0bits.AD1IE = 1; //enable ADC interrupt
    IPC3bits.AD1IP = 0x3; //set ADC interrupt priority
    IFS0bits.AD1IF = 0; //clear ADC interrupt flag
    AD1CON1bits.ADON = 1; //turn on ADC
    
    TRISAbits.TRISA0 = 0;   //configure RA0 (speaker input) as output
    TRISAbits.TRISA3 = 0;   //configure RA3 (motor driver input 1) as output
    TRISAbits.TRISA4 = 0;   //configure RA4 (motor driver input 2) as output
    AD1PCFGbits.PCFG9 = 1;
    TRISBbits.TRISB15 = 0;  //configure RB15 (light relay switch input) as output
    
    //configure OC2
    
//    config_OC2();   
//    OC2RS = 2000;               //set period
//    OC2R = (int)OC2RS/2;          //set duty
//    __delay_ms(1000);
    
//    OC2CON1bits.OCM = 0;      //turn off OC2
    
    struct tm timetest;
    timetest.tm_year = 19;
    timetest.tm_mon = 5;
    timetest.tm_mday = 23;
    timetest.tm_wday = 4;
    timetest.tm_hour = 6;
    timetest.tm_min = 30;
    timetest.tm_sec = 1;
        
    rtcc_settime(timetest);
    
    keypad_colflag = 0;
    keypad_rowflag = 0;
    keypad_pressflag = 0;
    keypad_numpressed = -1;
    
    format_year = 0;
    format_month = 0;
    format_time = 1;
    
    lcd_clear();
    lcd_printstring("Initializing...");
    __delay_ms(1000);
    lcd_clear();
    
    screenSelect = 1;
    goto screenHome1;

    screenHome1: //screenSelect = 1
    {
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        struct tm timenew;
        int dayold = 0;
        int done1 = 0, done2 = 0, done3 = 0, done4 = 0;
        lcd_clear();
        rtcc_printdate(NULL);
        lcd_moveline(2);
        rtcc_printtime(NULL, 0);
        
        

        while (screenSelect == 1) {   
            
            rtcc_gettime(&timenew);
            
            if (timenew.tm_mday != dayold) {
                lcd_clear();
                rtcc_printdate(NULL); 
            }
            lcd_moveline(2);
            rtcc_printtime(NULL, 0);
            lcd_hidecursor();
            __delay_ms(100);
            
            dayold = timenew.tm_mday;
            
            if ((timelighton.tm_hour == timenew.tm_hour) && (timelighton.tm_min == timenew.tm_min) &&  (done1 == 0)){
                lcd_clear();
                lcd_printstring("Turning on light...");
                __delay_ms(2000);
//                LATBbits.LATB15 = 1;
//                LATAbits.LATA0 = 1;
                lcd_clear();
                rtcc_printdate(NULL);
                lcd_moveline(2);
                rtcc_printtime(NULL, 0);
                done1 = 1;
            }
            
            if ((timelightoff.tm_hour == timenew.tm_hour) && (timelightoff.tm_min == timenew.tm_min) &&  (done2 == 0)){
                lcd_clear();
                lcd_printstring("Turn off light...");
                __delay_ms(2000);
//                LATBbits.LATB15 = 0;
//                LATAbits.LATA0 = 0;
                lcd_clear();
                rtcc_printdate(NULL);
                lcd_moveline(2);
                rtcc_printtime(NULL, 0);
                done2 = 1;
            }
            
            if ((timecurtainopen.tm_hour == timenew.tm_hour) && (timecurtainopen.tm_min == timenew.tm_min) &&  (done3 == 0)){
                lcd_clear();
                lcd_printstring("Opening curtains...");
                LATAbits.LATA3 = 1;
                LATAbits.LATA4 = 0;
                __delay_ms(5000);
                LATAbits.LATA3 = 0;
                LATAbits.LATA4 = 0;
                lcd_clear();
                rtcc_printdate(NULL);
                lcd_moveline(2);
                rtcc_printtime(NULL, 0);
                done3 = 1;
            }
            
            if ((timecurtainclose.tm_hour == timenew.tm_hour) && (timecurtainclose.tm_min == timenew.tm_min &&  (done4 == 0))){
                lcd_clear();
                lcd_printstring("Closing curtains...");
                LATAbits.LATA3 = 0;
                LATAbits.LATA4 = 1;
                __delay_ms(5000);
                LATAbits.LATA3 = 0;
                LATAbits.LATA4 = 0;
                lcd_clear();
                rtcc_printdate(NULL);
                lcd_moveline(2);
                rtcc_printtime(NULL, 0);
                done4 = 1;
            }
            
        }
        
        goto screenHome2;
    }
    
    screenHome2: //screenSelect = 2
    {
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        lcd_clear();
        lcd_printstring("1: Go to settings");
        lcd_moveline(2);
        lcd_printstring("2: Set alarm");
        lcd_hidecursor();
        
        while (screenSelect == 2) {
            while ((!keypad_pressflag) && (screenSelect == 2)) {
                keypad_cycle();
            }

            if (keypad_numpressed == 1){
                screenSelect = 5;
                break;
            } else if (keypad_numpressed == 2){
                screenSelect = 11;
                break;
            }           
            keypad_pressflag = 0;   
        }
        
        keypad_pressflag = 0;
        if (screenSelect == 5) goto screenSettings;
        else if (screenSelect == 11) goto screenSetAlarm1;
        
        screenSelect = 3;
        goto screenHome3;
    }
    
    screenHome3: //screenSelect = 3
    {
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        lcd_clear();
        lcd_printstring("3: Set light");
        lcd_moveline(2);
        lcd_printstring("4: Set curtains");
        lcd_hidecursor();
        
        while (screenSelect == 3) {
            while ((!keypad_pressflag) && (screenSelect == 3)) {
                keypad_cycle();
            }

            if (keypad_numpressed == 3){
                screenSelect = 8;
                break;
            } else if (keypad_numpressed == 4){
                screenSelect = 9;
                break;
            }           
            keypad_pressflag = 0;   
        }
        
        keypad_pressflag = 0;
        if (screenSelect == 8) goto screenSetLight;
        else if (screenSelect == 9) goto screenSetCurtains;
        
        screenSelect = 4;
        goto screenHome4;
    }
    
    screenHome4: //screenSelect = 4
    {
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        lcd_clear();
        lcd_printstring("5: View alarm");
        lcd_hidecursor();
        
        while (screenSelect == 4) {
            while ((!keypad_pressflag) && (screenSelect == 4)) {
                keypad_cycle();
            }

            if (keypad_numpressed == 5){
                screenSelect = 10;
                break;
            }    
            keypad_pressflag = 0;   
        }
        keypad_pressflag = 0;
        if (screenSelect == 10) goto screenViewAlarms;
        
        screenSelect = 1;
        goto screenHome1;
        
    }
    
    screenSettings: //screenSelect = 5
    {
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        lcd_clear();
        lcd_printstring("1:ChangeDateTime");
        lcd_moveline(2);
        lcd_printstring("2:DateTimeFormat");
        lcd_hidecursor();
        
        while (screenSelect == 5){
            while ((!keypad_pressflag) && (screenSelect == 5)) {
                keypad_cycle();
            }

            if (keypad_numpressed == 1){
                screenSelect = 6;
                break;
            } else if (keypad_numpressed == 2){
                screenSelect = 7;
                break;
            }           
            keypad_pressflag = 0;   
        }
        
        keypad_pressflag = 0;
        if (screenSelect == 6) goto screenSettingsCDT;
        else if (screenSelect == 7) goto screenSettingsDTF;
        
        lcd_clear();
        lcd_printstring("  Going back...    ");
        __delay_ms(1000);
        lcd_clear();
        
        screenSelect = 1;
        goto screenHome1;
    }
    
    screenSettingsCDT: //screenSelect = 6
    {
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        changeThis = 0;
        
        lcd_clear();
        rtcc_printdate(NULL);
        lcd_moveline(2);
        rtcc_printtime(NULL, 0);
        lcd_movecursor(2, 14);
        lcd_printstring("OK");
        lcd_hidecursor();
        
        struct tm timechange;
        rtcc_gettime(&timechange);
        
        float buffval = 0;  //stores relative ADC value
        
        while (screenSelect == 6) {
            while (changeThis == 0) {   //change month 
                buffval = 13*((float)(adcvalue-40)/1023);
                timechange.tm_mon = ceil(buffval) - 1;
                if (timechange.tm_mon < 1) timechange.tm_mon = 1;
                lcd_moveline(1);
                rtcc_printdate(&timechange);
                lcd_movecursor(1, 3);
                __delay_ms(50);
            }
            
            while (changeThis == 1) {   //change month day
                buffval = (monthdays[timechange.tm_mon-1]+1)*((float)(adcvalue-40)/1023);
                timechange.tm_mday = ceil(buffval);
                 if (timechange.tm_mday < 1) timechange.tm_mday = 1;
                lcd_moveline(1);
                rtcc_printdate(&timechange);
                lcd_movecursor(1, 0);
                __delay_ms(50);
            }
            
            while (changeThis == 2) {   //change year
                buffval = 100*((float)(adcvalue-15)/1023);
                timechange.tm_year = ceil(buffval)-1;
                if (timechange.tm_year < 0) timechange.tm_year = 0;
                lcd_moveline(1);
                rtcc_printdate(&timechange);
                lcd_movecursor(1, 7);
                __delay_ms(50);
            }
            
            while (changeThis == 3) {   //change weekday
                buffval = 7*((float)(adcvalue-40)/1023);
                timechange.tm_wday = ceil(buffval) - 1;
                lcd_moveline(1);
                rtcc_printdate(&timechange);
                lcd_movecursor(1, 13);
                __delay_ms(50);
            }
            
            while (changeThis == 4) {   //change hour
                buffval = 24*((float)(adcvalue-40)/1023);
                timechange.tm_hour = ceil(buffval) - 1;
                if (timechange.tm_hour < 0) timechange.tm_hour = 0;
                lcd_moveline(2);
                rtcc_printtime(&timechange, 0);
                lcd_movecursor(2, 2);
                __delay_ms(50);
            }
            
            while (changeThis == 5) {   //change min
                buffval = 60*((float)(adcvalue-15)/1023);
                timechange.tm_min = ceil(buffval) - 1;
                if (timechange.tm_min < 0) timechange.tm_min = 0;
                lcd_moveline(2);
                rtcc_printtime(&timechange, 0);
                lcd_movecursor(2, 5);
                __delay_ms(50);
            }
            
            while (changeThis == 6) {
                lcd_movecursor(2, 14);
                lcd_printstring("OK");
                lcd_movecursor(2, 14);
                __delay_ms(50);
            }
            
            rtcc_settime(timechange);  
        }
        
        rtcc_settime(timechange);
        
        lcd_clear();
        lcd_printstring("DateTime changed! ");
        __delay_ms(1000);

        screenSelect = 1;
        goto screenHome1;
    }
    
    screenSettingsDTF: //screenSelect = 7
    {
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        changeThis = 0;
        
        lcd_clear();
        rtcc_printdate(NULL);
        lcd_moveline(2);
        rtcc_printtime(NULL, 0);
        lcd_hidecursor();
        
        float buffval = 0;  //stores relative ADC value
        while (screenSelect == 7) {
            while (changeThis == 0) {
                buffval = 2*((float)adcvalue/1023);
                if (buffval < 1) {
                    format_month = 0;
                } else {
                    format_month = 1;
                }
                lcd_moveline(1);
                rtcc_printdate(NULL);
                lcd_movecursor(1, 3);
                __delay_ms(50);
            }
            
            while (changeThis == 1) {
                buffval = 2*((float)adcvalue/1023);
                if (buffval < 1) {
                    format_year = 0;
                } else if (buffval <= 2) {
                    format_year = 1;
                }
                lcd_moveline(1);
                rtcc_printdate(NULL);
                lcd_movecursor(1, 7);
                __delay_ms(50);
            }
            
            while (changeThis == 2) {
                buffval = 2*((float)adcvalue/1023);
                if (buffval < 1) {
                    format_time = 0;
                } else if (buffval <= 2) {
                    format_time = 1;
                }
                lcd_moveline(2);
                rtcc_printtime(NULL, 0);
                lcd_movecursor(2, 2);
                __delay_ms(50);
            }
        }
        
        lcd_clear();
        lcd_printstring("Format changed!");
        lcd_hidecursor();
        __delay_ms(1000);
        lcd_clear();
        
        screenSelect = 1;
        goto screenHome1;
    }
    
    screenSetLight: //screenSelect = 8
    {
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        changeThis = 0;
        
        lcd_clear();
        lcd_printstring("LtOn: ");
        lcd_movecursor(1, 6);
        rtcc_printtime(&timelighton, 0);
        lcd_moveline(2);
        lcd_printstring("LtOff:");
        lcd_movecursor(2, 6);
        rtcc_printtime(&timelightoff, 0);
        lcd_hidecursor();
        
        float buffval = 0;
        while (screenSelect == 8) {
            while (changeThis == 0) {   //change light on hour
                buffval = 24*((float)(adcvalue-40)/1023);
                timelighton.tm_hour = ceil(buffval) - 1;
                if (timelighton.tm_hour < 0) timelighton.tm_hour = 0;
                lcd_moveline(1);
                lcd_printstring("LtOn: ");
                lcd_movecursor(1, 6);
                rtcc_printtime(&timelighton, 0);
                lcd_movecursor(1, 8);
                __delay_ms(100);
            }
            
            while (changeThis == 1) {   //change light on min
                buffval = 60*((float)(adcvalue-15)/1023);
                timelighton.tm_min = ceil(buffval) - 1;
                if (timelighton.tm_min < 0) timelighton.tm_min = 0;
                lcd_moveline(1);
                lcd_printstring("LtOn: ");
                lcd_movecursor(1, 6);
                rtcc_printtime(&timelighton, 0);
                lcd_movecursor(1, 11);
                __delay_ms(100);
            }
            
            while (changeThis == 2) {   //change light off hour
                buffval = 24*((float)(adcvalue-40)/1023);
                timelightoff.tm_hour = ceil(buffval) - 1;
                if (timelightoff.tm_hour < 0) timelightoff.tm_hour = 0;
                lcd_moveline(2);
                lcd_printstring("LtOff:");
                lcd_movecursor(2, 6);
                rtcc_printtime(&timelightoff, 0);
                lcd_movecursor(2, 8);
                __delay_ms(100);
            }
            
            while (changeThis == 3) {   //change light off min
                buffval = 60*((float)(adcvalue-15)/1023);
                timelightoff.tm_min = ceil(buffval) - 1;
                if (timelightoff.tm_min < 0) timelightoff.tm_min = 0;
                lcd_moveline(2);
                lcd_printstring("LtOff:");
                lcd_movecursor(2, 6);
                rtcc_printtime(&timelightoff, 0);
                lcd_movecursor(2, 11);
                __delay_ms(100);
            }
        }
        
        lcd_clear();
        lcd_printstring("   Lights set!    ");
        __delay_ms(1000);
        lcd_clear();
        
        screenSelect = 1;
        goto screenHome1;
    }
    
    screenSetCurtains: //screenSelect = 9
    {
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        changeThis = 0;
        
        lcd_clear();
        lcd_printstring("CtOpn:");
        lcd_movecursor(1, 6);
        rtcc_printtime(&timecurtainopen, 0);
        lcd_moveline(2);
        lcd_printstring("CtCls:");
        lcd_movecursor(2, 6);
        rtcc_printtime(&timecurtainclose, 0);
        lcd_hidecursor();
        
        float buffval = 0;
        while (screenSelect == 9) {
            while (changeThis == 0) {   //change curtain open hour
                buffval = 24*((float)(adcvalue-40)/1023);
                timecurtainopen.tm_hour = ceil(buffval) - 1;
                if (timecurtainopen.tm_hour < 0) timecurtainopen.tm_hour = 0;
                lcd_moveline(1);
                lcd_printstring("CtOpn:");
                lcd_movecursor(1, 6);
                rtcc_printtime(&timecurtainopen, 0);
                lcd_movecursor(1, 8);
                __delay_ms(100);
            }
            
            while (changeThis == 1) {   //change curtain open min
                buffval = 60*((float)(adcvalue-15)/1023);
                timecurtainopen.tm_min = ceil(buffval) - 1;
                if (timecurtainopen.tm_min < 0) timecurtainopen.tm_min = 0;
                lcd_moveline(1);
                lcd_printstring("CtOpn:");
                lcd_movecursor(1, 6);
                rtcc_printtime(&timecurtainopen, 0);
                lcd_movecursor(1, 11);
                __delay_ms(100);
            }
            
            while (changeThis == 2) {   //change curtain close hour
                buffval = 24*((float)(adcvalue-40)/1023);
                timecurtainclose.tm_hour = ceil(buffval) - 1;
                if (timecurtainclose.tm_hour < 0) timecurtainclose.tm_hour = 0;
                lcd_moveline(2);
                lcd_printstring("CtCls:6");
                lcd_movecursor(2, 6);
                rtcc_printtime(&timecurtainclose, 0);
                lcd_movecursor(2, 8);
                __delay_ms(100);
            }
            
            while (changeThis == 3) {   //change curtain close min
                buffval = 60*((float)(adcvalue-15)/1023);
                timecurtainclose.tm_min = ceil(buffval) - 1;
                if (timecurtainclose.tm_min < 0) timecurtainclose.tm_min = 0;
                lcd_moveline(2);
                lcd_printstring("CtCls:");
                lcd_movecursor(2, 6);
                rtcc_printtime(&timecurtainclose, 0);
                lcd_movecursor(2, 11);
                __delay_ms(100);
            }
        }
        
        lcd_clear();
        lcd_printstring("  Curtains set!    ");
        __delay_ms(1000);
        lcd_clear();
        
        screenSelect = 1;
        goto screenHome1;
    }
    
    screenViewAlarms: //screenSelect = 10
    {
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        lcd_clear();
        if (timealarm.tm_mday != -1)
            sprintf(line1, "%s %02d %s %02d:%02d", getmonth(timealarm.tm_mon), timealarm.tm_mday, getweekday(timealarm.tm_wday), timealarm.tm_hour, timealarm.tm_min);
        else
            sprintf(line1, "%s -- %s %02d:%02d", getmonth(timealarm.tm_mon), getweekday(timealarm.tm_wday), timealarm.tm_hour, timealarm.tm_min);
        lcd_printstring(line1);
        lcd_moveline(2);
        lcd_printstring(alarmreminder);
        lcd_hidecursor();
        
        __delay_ms(4000);
        
        lcd_clear();
        lcd_printstring("  Going back...    ");
        __delay_ms(1000);
        lcd_clear();
        
        screenSelect = 1;
        goto screenHome1;
    }
    
    screenSetAlarm1: //screenSelect = 11
    {
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        changeThis = 0;
        
        lcd_clear();
        
        float buffval = 0;  //stores relative ADC value
        int alarmmask = 0;
        while (screenSelect == 11) {
            while (changeThis == 0) {   //change month 
                buffval = 13*((float)(adcvalue-40)/1023);
                timealarm.tm_mon = ceil(buffval) - 1;
                if (timealarm.tm_mon < 1) timealarm.tm_mon = -1;
                lcd_moveline(1);
                sprintf(line1, "Alarm:%s %02d %s", getmonth(timealarm.tm_mon), timealarm.tm_mday, getweekday(timealarm.tm_wday));
                lcd_printstring(line1);
                lcd_movecursor(1, 6);
                __delay_ms(50);
            }
            
            while (changeThis == 1) {   //change month day
                if (timealarm.tm_mon != -1)
                    buffval = (monthdays[timealarm.tm_mon-1]+1)*((float)(adcvalue-40)/1023);
                else
                    buffval = (32)*((float)(adcvalue-40)/1023);
                timealarm.tm_mday = ceil(buffval);
                if (timealarm.tm_mday < 1) timealarm.tm_mday = -1;
                lcd_moveline(1);
                if (timealarm.tm_mday != -1)
                    sprintf(line1, "Alarm:%s %02d %s", getmonth(timealarm.tm_mon), timealarm.tm_mday, getweekday(timealarm.tm_wday));
                else
                    sprintf(line1, "Alarm:%s -- %s", getmonth(timealarm.tm_mon), getweekday(timealarm.tm_wday));
                lcd_printstring(line1);
                lcd_movecursor(1, 10);
                __delay_ms(50);
            }
            
            while (changeThis == 2) {   //change weekday
                buffval = 7*((float)(adcvalue-40)/1023);
                timealarm.tm_wday = ceil(buffval) - 1;
                if (timealarm.tm_wday < 0) timealarm.tm_wday = -1;
                lcd_moveline(1);
                if (timealarm.tm_mday != -1)
                    sprintf(line1, "Alarm:%s %02d %s", getmonth(timealarm.tm_mon), timealarm.tm_mday, getweekday(timealarm.tm_wday));
                else
                    sprintf(line1, "Alarm:%s -- %s", getmonth(timealarm.tm_mon), getweekday(timealarm.tm_wday));
                lcd_printstring(line1);
                lcd_movecursor(1, 13);
                __delay_ms(50);
            }
            
            while (changeThis == 3) {   //change hour
                buffval = 24*((float)(adcvalue-40)/1023);
                timealarm.tm_hour = ceil(buffval) - 1;
                if (timealarm.tm_hour < 0) timealarm.tm_hour = 0;
                lcd_moveline(2);
                rtcc_printtime(&timealarm, 0);
                lcd_movecursor(2, 2);
                __delay_ms(50);
            }
            
            while (changeThis == 4) {   //change min
                buffval = 60*((float)(adcvalue-15)/1023);
                timealarm.tm_min = ceil(buffval) - 1;
                if (timealarm.tm_min < 0) timealarm.tm_min = 0;
                lcd_moveline(2);
                rtcc_printtime(&timealarm, 0);
                lcd_movecursor(2, 5);
                __delay_ms(50);
            }
            
            if ((timealarm.tm_mday == -1) && (timealarm.tm_mon == -1) && (timealarm.tm_wday == -1)) alarmmask = 0b0110; // alarm every day
            else if ((timealarm.tm_mday == -1) && (timealarm.tm_mon == -1)) alarmmask = 0b0111; // alarm every week
            else if ((timealarm.tm_mon == -1) && (timealarm.tm_wday == -1)) alarmmask = 0b1000; // alarm every month
            else if ((timealarm.tm_mday != -1) && (timealarm.tm_mon != -1) && (timealarm.tm_wday != -1)) alarmmask = 0b1001; // alarm every year
            else alarmmask = 0b0110; // alarm every day
            
            rtcc_setalarm(timealarm, alarmmask);     
        }
        rtcc_setalarm(timealarm, alarmmask);    
        lcd_clear();
        lcd_printstring("Alarm time set...");
        __delay_ms(1000);
        
        screenSelect = 12;
        goto screenSetAlarm2;
        
    }
   
    screenSetAlarm2: //screenSelect = 12
    {
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        lcd_clear();
        lcd_printstring("Alarm sound:");
        lcd_moveline(2);
        lcd_printstring("1      2       ");
        
        float buffval = 0;
        while (screenSelect == 12) {
            buffval = 2*((float)(adcvalue-15)/1023);
            if (buffval < 1){
                lcd_movecursor(2, 0);
                alarmsound = 1;
                //play Alarm 1 sound
                config_OC2();   
                OC2RS = 4000;               //set period
                OC2R = (int)OC2RS/2;          //set duty
                __delay_ms(200);
                OC2RS = 3000;               //set period
                OC2R = (int)OC2RS/2;          //set duty
                __delay_ms(200);
                OC2RS = 2000;               //set period
                OC2R = (int)OC2RS/2;          //set duty
                __delay_ms(200);
                OC2RS = 1000;               //set period
                OC2R = (int)OC2RS/2;          //set duty
                __delay_ms(200);
                OC2CON1bits.OCM = 0;        //turn off OC2
                __delay_ms(250);
                
            } else {
                lcd_movecursor(2, 7);
                alarmsound = 2;
                //play Alarm 2 sound
                config_OC2();   
                OC2RS = 2000;               //set period
                OC2R = (int)OC2RS/2;          //set duty
                __delay_ms(200);
                OC2RS = 4000;               //set period
                OC2R = (int)OC2RS/2;          //set duty
                __delay_ms(200);
                OC2RS = 1000;               //set period
                OC2R = (int)OC2RS/2;          //set duty
                __delay_ms(200);
                OC2RS = 3000;               //set period
                OC2R = (int)OC2RS/2;          //set duty
                __delay_ms(200);
                OC2CON1bits.OCM = 0;        //turn off OC2
                __delay_ms(250);
            }
        }
        
        screenSelect = 13;
        goto screenSetAlarm3;
    }
    
    screenSetAlarm3: //screenSelect = 13
    {
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        lcd_clear();
        lcd_printstring("Alarm reminder: ");
        lcd_moveline(2);
        strcpy(alarmreminder, "                ");
        
        int index = 0, presses = 0, keypad_numpressed_old;
        while (screenSelect == 13) {    //alphanumeric keypad function! 1-abc, 2-def, 3-ghi, 4-jkl, 5-mno, 6-pqr, 7-stu, 8-vwx, 9-yz
            while(!keypad_pressflag){
                keypad_cycle();
                if(screenSelect != 13) break;
            }
            
            keypad_pressflag = 0;
            
            if((keypad_numpressed != -1) && (keypad_numpressed != 11)){
                if(keypad_numpressed == -3){        // # key functions as "next index" so you can type the next letter
                    presses = 0;
                    index++;
                    alarmreminder[(index)] = 32;
                } else if (presses == 0){
                    alarmreminder[index] = (keypad_numpressed + 48);    //map the number pressed to the sets of letters associated with them
                    keypad_numpressed_old = keypad_numpressed;
                    presses++;
                } else if (keypad_numpressed_old == keypad_numpressed){
                    alarmreminder[index] = ((keypad_numpressed*3)-2) + (48*2) + (presses-1);    //map the number pressed to the sets of letters associated with them
                    presses++;
                    if (keypad_numpressed == 9) {
                        if (presses == 3) presses = 0;
                    }
                    else
                        if (presses == 4) presses = 0;
                }
                
            } else if (keypad_numpressed == 11){    // * key functions as backspace
                alarmreminder[(index)] = 32;
                index--;
                if (index < 0) index = 0;
                presses = 0;
            }
            lcd_moveline(2);
            lcd_printstring(alarmreminder);
            lcd_movecursor(2, index);
            keypad_numpressed = -1;
            __delay_ms(50);
            if((index >= 16) || (screenSelect != 13)) break;
        }
        
        lcd_clear();
        lcd_printstring("   Alarm set!    ");
        __delay_ms(1000);
        lcd_clear();
        
        screenSelect = 1;
        goto screenHome1;
    }
    
    return 0;
}

void __attribute__ ((interrupt)) _CNInterrupt(void) {
    static int debounceCounter = 0;
    
    debounceCounter = 0;
    
    if (!PORTAbits.RA2){
        while ((!PORTAbits.RA2) && (debounceCounter < DEBOUNCEMAX)){ 
            debounceCounter++;
        }
        
        if((debounceCounter == DEBOUNCEMAX) && ((screenSelect < 6) || (screenSelect > 11))){
            screenSelect++;
        } else if((debounceCounter == DEBOUNCEMAX) && (screenSelect == 6)){
            if (changeThis == 6) {
                screenSelect++;
                changeThis++;
            }
            changeThis++;
        } else if((debounceCounter == DEBOUNCEMAX) && (screenSelect == 7)){
            if (changeThis == 2) {
                screenSelect++;
                changeThis++;
            }
            changeThis++;
        } else if((debounceCounter == DEBOUNCEMAX) && (screenSelect == 8)){
            if (changeThis == 3) {
                screenSelect++;
                changeThis++;
            }
            changeThis++;
        } else if((debounceCounter == DEBOUNCEMAX) && (screenSelect == 9)){
            if (changeThis == 3) {
                screenSelect++;
                changeThis++;
            }
            changeThis++;
        } else if((debounceCounter == DEBOUNCEMAX) && (screenSelect == 11)){
            if (changeThis == 4) {
                screenSelect++;
                changeThis++;
            }
            changeThis++;
        }
    }
    else if (!PORTBbits.RB7){
        while ((!PORTBbits.RB7) && (debounceCounter < DEBOUNCEMAX)){ 
            debounceCounter++;
        }
        
        while((!PORTBbits.RB7))
            Nop();
        
        if((debounceCounter == DEBOUNCEMAX)){
            keypad_rowflag = 1;
            keypad_pressflag = 1;
        } 
        else
            keypad_rowflag = 0;
        
    }
    else if (!PORTBbits.RB8){
        while ((!PORTBbits.RB8) && (debounceCounter < DEBOUNCEMAX)){ 
            debounceCounter++;
        }
        
        while((!PORTBbits.RB8))
            Nop();
        
        if((debounceCounter == DEBOUNCEMAX)){
            keypad_rowflag = 2;
            keypad_pressflag = 1;
        } 
        else
            keypad_rowflag = 0;
        
    }
    else if (!PORTBbits.RB9){
        while ((!PORTBbits.RB9) && (debounceCounter < DEBOUNCEMAX)){ 
            debounceCounter++;
        }
        
        while((!PORTBbits.RB9))
            Nop();
        
        if((debounceCounter == DEBOUNCEMAX)){
            keypad_rowflag = 3;
            keypad_pressflag = 1;
        } 
        else
            keypad_rowflag = 0;
        
    }
    else if (!PORTBbits.RB10){
        while ((!PORTBbits.RB10) && (debounceCounter < DEBOUNCEMAX)){ 
            debounceCounter++;
        }
        
        while((!PORTBbits.RB10))
            Nop();
        
        if((debounceCounter == DEBOUNCEMAX)){
            keypad_rowflag = 4;
            keypad_pressflag = 1;
        } 
        else
            keypad_rowflag = 0; 
    }
    
    IFS1bits.CNIF = 0; // clear flag
    if(keypad_pressflag){
        keypad_parsepress();
    }
}

void __attribute__ ((interrupt, no_auto_psv)) _ADC1Interrupt(void) {
    IEC0bits.AD1IE = 0; //disable interrupt
    IFS0bits.AD1IF = 0; //clear flag
    
    //copy ADC output to adcvalue
    adcvalue = ADC1BUF0;
            
    IEC0bits.AD1IE = 1; //enable interrupt
    IFS0bits.AD1IF = 0; //clear flag
    
}

void __attribute__((interrupt,no_auto_psv)) _RTCCInterrupt(void) {
    IFS3bits.RTCIF = 0;     // clear flag
    
    lcd_clear();
    lcd_printstring("    Alarm: ");
    lcd_moveline(2);
    lcd_printstring(alarmreminder);
    
    static int i = 0;
    
    for(i = 0; i < 3; i++){
        if (alarmsound == 1){
            config_OC2(); 
            OC2RS = 4000;               //set period
            OC2R = (int)OC2RS/2;          //set duty
            __delay_ms(200);
            OC2RS = 3000;               //set period
            OC2R = (int)OC2RS/2;          //set duty
            __delay_ms(200);
            OC2RS = 2000;               //set period
            OC2R = (int)OC2RS/2;          //set duty
            __delay_ms(200);
            OC2RS = 1000;               //set period
            OC2R = (int)OC2RS/2;          //set duty
            __delay_ms(200);
            OC2CON1bits.OCM = 0;        //turn off OC2
        } else if (alarmsound == 2){
            config_OC2();   
            OC2RS = 2000;               //set period
            OC2R = (int)OC2RS/2;          //set duty
            __delay_ms(200);
            OC2RS = 4000;               //set period
            OC2R = (int)OC2RS/2;          //set duty
            __delay_ms(200);
            OC2RS = 1000;               //set period
            OC2R = (int)OC2RS/2;          //set duty
            __delay_ms(200);
            OC2RS = 3000;               //set period
            OC2R = (int)OC2RS/2;          //set duty
            __delay_ms(200);
            OC2CON1bits.OCM = 0;        //turn off OC2
        }
    }
    __delay_ms(1500);
    lcd_clear();
    rtcc_printdate(NULL);
    lcd_moveline(2);
    rtcc_printtime(NULL, 0);
}

void config_OC2(void){
    TRISAbits.TRISA0 = 0; //set RA0 is output
    
    __builtin_write_OSCCONL((OSCCON) | 0xBF); // clears IOLOCK bit
    _RP5R = 19; //assign OC1 to RP5
    __builtin_write_OSCCONL((OSCCON) | 0x40); // sets IOLOCK bit
    
    OC2CON1 = 0;
    OC2CON2 = 0;
    OC2CON1bits.OCTSEL = 0x07;
    OC2RS = 199;    //set period
    OC2R = (int)(OC2RS+1)/2;    //set duty
    OC2CON1bits.OCM = 6;
    OC2CON2bits.SYNCSEL=0x1F;
}



