/*
 * File:   adc_lib.h
 * Author: kirsten_hipolitoEEE
 *
 * Created on March 6, 2019, 10:07 AM
 */

#include "xc.h"
#define FCY 4000000UL
#include "libpic30.h"

int adcvalue;

void adc_initialize(){
    //set A/D Port Configuration Register
    AD1PCFG = 0xFFFD; //configure as 1111 1111 1111 1101;
    TRISAbits.TRISA1 = 1; // set RA1 = AN1 to input

    //set A/D Control Register 1
    AD1CON1bits.ADSIDL = 1; //enable stop in idle mode
    AD1CON1bits.FORM = 0b00; //output in integer form
    AD1CON1bits.SSRC = 0b111; //internal counter trigger
    AD1CON1bits.ASAM = 1; //autostart sampling
    AD1CON1bits.SAMP = 1; //enable sampling

    //set A/D Control Register 2
    AD1CON2bits.CSCNA = 0; //do not scan inputs
    AD1CON2bits.SMPI = 0x0; //interrupt at completion of conversion for each sample/convert
    AD1CON2bits.BUFM = 0; //buffer as one 16 word buffer
    AD1CON2bits.ALTS = 0; //always use muxA

    //set A/D Control Register 3
    AD1CON3bits.ADRC = 0; //clock from system clock
    AD1CON3bits.SAMC = 0x02; // set auto sample time 2*TAD
    AD1CON3bits.ADCS = 0x01; //set A/D clock conversion to 2*TCY

    //set A/D Input Select
    AD1CHS = 0x0000; //set all bits to zero;
    AD1CHSbits.CH0SA = 0b00001; //channel 0 +input is AN1

    //set Input Scan Select Register
    AD1CSSL = 0x0000; //disable input scan
    }
