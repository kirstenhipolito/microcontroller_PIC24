/*
 * File:   lcd_lib.h
 * Author: kirstenhipolito_EEE
 *
 * Created on March 6, 2019, 10:07 AM
 */

/*
 * A library of dot matrix LCD functions to make LCD I/O more streamlined.
 * Made by: Kirsten Rae C. Hipolito
 * 2019 March
 * 
 * Assumes E connected to RB5, RS to RB4, D[7:4] to RB[3:0]
 */

#include "xc.h"
#include "string.h"
#define FCY 4000000UL
#include "libpic30.h"

void lcd_initialize(void);
//initialize LCD

void lcd_printstring(char* datastring);
//print input data string to LCD

void lcd_readbyte(void);
//read byte from LCD

void lcd_sendbyte(int data, int op_type);
//send command to LCD
/*op_type:
 0: 4bit command
 1: 8bit command
 2: 8bit character*/

void lcd_clear(void);
//clear LCD screen

void lcd_movecursor(int line_num, int csr_loc);
//move cursor location in LCD
/*line_num:
 1: line 1 (line above)
 2: line 2 (line below)*/
/*csr_loc: (note this only takes up to 0b00111111,
  and since there are only 16 boxes, only up to csr_loc = 15 will be visible
 0: first box
 15: last box*/

void lcd_hidecursor(void);
//hides cursor from view

void lcd_moveline(int line_num);
//move the line to print in
/*line_num:
 1: line 1 (line above)
 2: line 2 (line below)*/

void lcd_send4bits(int data);
//send 4bits to LCD

void lcd_send8bits(int data);
//send 8bits to LCD


void lcd_initialize(void){
    AD1PCFG = AD1PCFG | (0x003F);
    TRISB = TRISB & (0xFFC0);
    
    __delay_ms(15); //call 15ms delay
    lcd_sendbyte(0x30, 0);
    __delay_ms(4.1);
    lcd_sendbyte(0x30, 0);
    __delay_us(100);
    lcd_sendbyte(0x30, 0); //set function set command 3
    lcd_sendbyte(0x20, 0); //set to 4bit
    lcd_sendbyte(0x28, 1); //set function set
    lcd_sendbyte(0x08, 1); //display OFF
    lcd_sendbyte(0x01, 1); //clear display
    lcd_sendbyte(0x06, 1); //entry mode set
    lcd_sendbyte(0x0F, 1); //display ON
    lcd_sendbyte(0x06, 1); //entry mode set
}

void lcd_clear(void){
    lcd_sendbyte(0x01, 1);
}

void lcd_movecursor(int line_num, int csr_loc){
    int  addr_csr = 0b10000000;
    
    //check what line num, and set addr_csr 0b1X000000 accordingly
    if (line_num == 1){
        addr_csr = 0b10000000;
    } else if (line_num == 2){
        addr_csr = 0b11000000;
    }
    
    //translate csr_loc to cursor address in line
    addr_csr = addr_csr | ((0b00111111)&csr_loc);
    
    //send command to LCD
    lcd_sendbyte(addr_csr, 1);
    
}

void lcd_hidecursor(void){
    lcd_movecursor(1,16);
}

void lcd_moveline(int line_num){
    if (line_num == 1){
        lcd_sendbyte(0b10000000, 1);
    }
    else if (line_num == 2){
        lcd_sendbyte(0b11000000, 1);
    }
}

void lcd_printstring(char* datastring){
    int datalen = 0;
    int i = 0;
    
    datalen = strlen(datastring);
    
    for(i=0; i<datalen; i++){
        lcd_sendbyte(datastring[i], 2);
    }
}

void lcd_sendbyte(int data, int op_type){
    int charmask = 0x00;
    
    charmask = ((data >> 4) & 0x0F); // get upper 4 bits of data and write to charmask
    
    if(op_type!=2){
        LATBbits.LATB4 = 0; //set RB[4] = RS to 0 (IR)
        charmask = charmask & (0b11101111);
    } else if(op_type==2){
        LATBbits.LATB4 = 1;
        charmask = charmask | (0b00010000);
    }
    __delay_us(250);
    
    LATBbits.LATB5 = 1; //set RB[5] = E to 1
    charmask = charmask | (0b00100000);
    __delay_us(250);
    
    LATB = charmask; //write the contents of charmask to LATB
    __delay_us(250);
    
    LATBbits.LATB5 = 0; //set RB[5] = E to 0
    __delay_us(250);
    
    __delay_us(250);
    
    if (op_type == 0){
        goto end;
    }
    
    charmask = 0;
    
    //send lower nibble
    charmask = (data & 0x0F); // get upper 4 bits of commanddata and write to charmask
    
    if(op_type!=2){
        LATBbits.LATB4 = 0; //set RB[4] = RS to 0 (IR)
        charmask = charmask & (0b11101111);
    } else if(op_type==2){
        LATBbits.LATB4 = 1;
        charmask = charmask | (0b00010000);
    }
    __delay_us(250);
    
    LATBbits.LATB5 = 1; //set RB[5] = E to 1
    charmask = charmask | (0b00100000);
    __delay_us(250);
    
    LATB = charmask; //write the contents of charmask to LATB
    __delay_us(250);
    
    LATBbits.LATB5 = 0; //set RB[5] = E to 0
    __delay_us(250);
    
    charmask = 0;
    __delay_us(250);
    
end:
    __delay_us(250);
}

void lcd_send4bits(int data){
    int charmask = 0;
    charmask = data;
    
    LATBbits.LATB4 = 0;
    charmask = charmask & (0b11101111);
    Nop();
    __delay_ms(15);
    
    LATBbits.LATB5 = 1;
    charmask = charmask | (0b00100000);
    Nop();
    __delay_ms(15);
    
    LATB = charmask;
    Nop();
    __delay_ms(15);
    
    LATBbits.LATB5 = 0;
    charmask = 0;
    
    __delay_us(100);
}

void lcd_send8bits(int data){
    int data4bit = 0;
    
    data4bit = data >> 4;
    data4bit =  (data & 0x0F);
    lcd_send4bits(data4bit);
    __delay_ms(15);
    data4bit = (data & 0x0F);
    lcd_send4bits(data4bit);
    __delay_ms(15);
}
