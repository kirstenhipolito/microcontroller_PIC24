/*
 * File:   keypad_lib.h
 * Author: kirstenhipolito_EEE
 *
 * Created on April 2, 2019, 4:50 PM
 */

/*
 * A library of keypad I/O functions.
 * Made by: Kirsten Rae C. Hipolito
 * 2019 April
 * 
 * Assumes Row[1:4] connected to RB[7:10], assigned as input
 * Assumes Col[1:3] to RB[11,13:14], assigned as output
 */

#include "xc.h"
#include "stdio.h"
#include "string.h"
#define FCY 4000000UL
#include "libpic30.h"

int keypad_colflag = 0;
int keypad_rowflag = 0;
int keypad_pressflag = 0;
int keypad_numpressed = -1;

void keypad_initialize(void);
//initialize keypad

void keypad_cycle(void);
//starts cyclically enabling the columns

void keypad_parsepress(void);

void keypad_parsepressAlphaN(void);

    
void keypad_initialize(void){
    //set input/output
    TRISBbits.TRISB7 = 1;       //set RB7 = CN23 = keypad row 1 as input
    TRISBbits.TRISB8 = 1;       //set RB8 = CN22 = keypad row 2 as input
    TRISBbits.TRISB9 = 1;       //set RB9 = CN21 = keypad row 3 as input
    TRISBbits.TRISB10 = 1;      //set RB10 = CN16 = keypad row 4 as input
    TRISBbits.TRISB11 = 0;      //set RB11 = keypad col 1 as output
    TRISBbits.TRISB13 = 0;      //set RB13 = keypad col 2 as output
    TRISBbits.TRISB14 = 0;      //set RB14 = keypad col 3 as output
    LATBbits.LATB11 = 1;        //set RB11 = 1, initially high
    LATBbits.LATB13 = 1;        //set RB13 = 1, initially high
    LATBbits.LATB14 = 1;        //set RB14 = 1, initially high
    
    //configure pull-up
    CNPU2bits.CN16PUE = 1;      //enable pull-up for RB10
    CNPU2bits.CN21PUE = 1;      //enable pull-up for RB9
    CNPU2bits.CN22PUE = 1;      //enable pull-up for RB8
    CNPU2bits.CN23PUE = 1;      //enable pull-up for RB7
    
    //configure change notification interrupt
    CNEN2bits.CN16IE = 1;       //enable CN for RB10
    CNEN2bits.CN21IE = 1;       //enable CN for RB9
    CNEN2bits.CN22IE = 1;       //enable CN for RB8
    CNEN2bits.CN23IE = 1;       //enable CN for RB7
    
}

void keypad_cycle(void){
    keypad_colflag = 1;
    LATBbits.LATB11 = 0; // pull down col1
    LATBbits.LATB13 = 1; // pull up col2
    LATBbits.LATB14 = 1; // pull up col3
    __delay_ms(20);
    
    keypad_colflag = 2;
    LATBbits.LATB11 = 1; // pull up col1
    LATBbits.LATB13 = 0; // pull down col2
    LATBbits.LATB14 = 1; // pull up col3
    __delay_ms(20);
    
    keypad_colflag = 3;
    LATBbits.LATB11 = 1; // pull up col1
    LATBbits.LATB13 = 1; // pull up col2
    LATBbits.LATB14 = 0; // pull down col3
    __delay_ms(20);
       
}

void keypad_parsepress(void){                 
    if(keypad_colflag == 1){
        switch(keypad_rowflag){
            case 1:
                keypad_numpressed = 1;
                break;
            case 2:
                keypad_numpressed = 4;
                break;
            case 3:
                keypad_numpressed = 7;
                break;
            case 4:
                keypad_numpressed = 11;
                break;
        }
    } 
    else if (keypad_colflag == 2){
        switch(keypad_rowflag){
            case 1:
                keypad_numpressed = 2;
                break;
            case 2:
                keypad_numpressed = 5;
                break;
            case 3:
                keypad_numpressed = 8;
                break;
            case 4:
                keypad_numpressed = 0;
                break;
        }
    } 
    else if (keypad_colflag == 3){
        switch(keypad_rowflag){
            case 1:
                keypad_numpressed = 3;
                break;
            case 2:
                keypad_numpressed = 6;
                break;
            case 3:
                keypad_numpressed = 9;
                break;
            case 4:
                keypad_numpressed = -3;
                break;
        }
    } 
    else {
        keypad_numpressed = -1;
    }
    
    keypad_colflag = 0;
    keypad_rowflag = 0;
    
}

void keypad_parsepressAlphaN(void){
    
}