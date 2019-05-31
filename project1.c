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
#include "lcd_lib.h"
#include "keypad_lib.h"

_CONFIG1 (FWDTEN_OFF & JTAGEN_OFF)
_CONFIG2 (POSCMOD_NONE & OSCIOFNC_ON & FCKSM_CSDCMD & FNOSC_FRCPLL & PLL96MHZ_OFF & PLLDIV_NODIV)
_CONFIG3 (SOSCSEL_IO)

#define DEBOUNCEMAX 100

void __attribute__ ((interrupt)) _CNInterrupt(void);
void config_OC2(void);

int gameMode = 0;
int gameDiff = 0;
int playerScore = 0;
int stateSel = 1;
int adcvalue;
int enterAns = 0;
int dutyflag = 0;
int pointNum = 0;
unsigned long int maxTime = 0;
int addToDuty = 0;
unsigned long int timerPassed = 0;

int main(void) {
    AD1PCFG = 0xFFFF;           //initialize all IO pins as digital
    lcd_initialize();           //initialize LCD using function in library
    TRISAbits.TRISA2 = 1;       //set RA2 as input (for pushbutton)
    TRISAbits.TRISA1 = 1;       //set RA1 as input (for ADC pot)
    TRISAbits.TRISA0 = 0;       //set RA0 as output (for LED)
    LATAbits.LATA0 = 1;         //turn off LED
    keypad_initialize();        //initialize keypad using function in library
    
    AD1PCFGbits.PCFG1 = 0;      //set RA1 = AN1 as analog (for ADC pot)
    
    
    //configure ADC
    //set A/D Control Register 1
    AD1CON1bits.ADSIDL = 1; //enable stop in idle mode
    AD1CON1bits.FORM = 0b00; //output in integer form
    AD1CON1bits.SSRC = 0b111; //internal counter trigger
    AD1CON1bits.ASAM = 1; //autostart sampling
    AD1CON1bits.SAMP = 1; //enable sampling
    
    //set A/D Control Register 2
    AD1CON2bits.CSCNA = 0; //do not scan inputs
    AD1CON2bits.SMPI = 0xF; //interrupt at completion of conversion for every 16th sample/convert
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
    
    //configure ADC
    IEC0bits.AD1IE = 1; //enable ADC interrupt
    IPC3bits.AD1IP = 0x3; //set ADC interrupt priority
    IFS0bits.AD1IF = 0; //clear ADC interrupt flag
    
    
    //configure  input change notification interrupt
    CNEN2bits.CN30IE = 1;       //enable CN for RA2 (pushbutton)
    IEC1bits.CNIE = 1;          //enable CN interrupt
    IPC4bits.CNIP = 0x5;        //set CN interrupt priority
    IFS1bits.CNIF = 0;          //clear CN interrupt flag
    
    
    //configure randomizer
    
    
    goto stateStart;
    
    //STATE 1: START OF GAME: Welcome screen with name
stateStart:
    {
        gameMode = 0;
        gameDiff = 1;
        playerScore = 0;
        stateSel = 1;
        adcvalue = 0;
        enterAns = 0;
        dutyflag = 0;
        maxTime = 0;
        addToDuty = 0;
        timerPassed = 0;
        pointNum = 0;
        OC2RS = 0x0F;                   //set period
        OC2R = 0xFF;
        OC2CON1 = 0;
        OC2CON2 = 0;
        LATAbits.LATA0 = 1;         //turn off LED
        lcd_clear();
        keypad_numpressed = -1;
        keypad_pressflag = 0;
        lcd_printstring("   WELCOME TO");
        lcd_moveline(2);
        lcd_printstring(" KIRSTEN'S GAME! ");
        
        while(stateSel == 1){
            Nop();
        }
        
        goto stateModeSel;
    }
    
    //STATE 2: MODE SELECTION: Mode Select, 1 second, Mode options
stateModeSel:
    {
        lcd_clear();
        lcd_printstring("   GAME MODE");
        lcd_moveline(2);
        lcd_printstring("     SELECT     ");
        
        __delay_ms(1000);
        lcd_clear();
        
        lcd_printstring("1-Key Input Mode");
        lcd_moveline(2);
        lcd_printstring("2-Multi Choice   ");
        
        while(stateSel == 2){
            while(!keypad_pressflag){
                keypad_cycle();
            }
            if(keypad_numpressed == 1){
                gameMode = 1;
                break;
            } else if (keypad_numpressed == 2){
                gameMode = 2;
                break;
            }
            keypad_pressflag = 0;
        }
        
        stateSel = 3;
        goto stateDiffSel;
        
    }
    
    //STATE 3: DIFFICULTY SELECTION: Game Mode __(gameMode)__ ; Difficulty __Easy/Ave?hard__
stateDiffSel:
    {
        AD1CON1bits.ADON = 1; //turn on ADC
        char modeStr[12];
        sprintf(modeStr, "Game Mode %d", gameMode);
        
        lcd_clear();
        lcd_printstring(modeStr);
        lcd_moveline(2);
        lcd_printstring("Difficulty: ");
        
        float buffgameDiff = 0;
        
        while(stateSel == 3){
            buffgameDiff = 3*((float)adcvalue/1023);
            if (buffgameDiff < 1){
                gameDiff = 1;
                lcd_movecursor(2, 12);
                lcd_printstring("Easy");
            }
            else if (buffgameDiff < 2){
                gameDiff = 2;
                lcd_movecursor(2, 12);
                lcd_printstring("Ave ");
            }
            else if (buffgameDiff < 3){
                gameDiff = 3;
                lcd_movecursor(2, 12);
                lcd_printstring("Hard");
            }
        }
        
        stateSel = 4;
        goto stateGame;
        
    }
    
    //STATE 4: GAME
stateGame:
    {
        playerScore = 0;
        srand(adcvalue);            //seed randomizer with adcvalue (degree of randomness)
        char quesStr[16] = "";
        char ansStr[16] = "";
        int opA = (rand() % 19) - 9; //to get [-9, 9]: generate a random int from [0, 18] then subtract 9
        int opB = (rand() % 19) - 9;
        int opNum = 0;
        int answer = 0;
        int letterAns = 0;
        int userAns = 0;
        char numAns[5] = "";
        int numans_i = 0;
        
        strcpy(numAns,"     "); //clear numAns string
        lcd_clear();
        
        //configure Timer2
        T2CON = 0x00;               //Stops any 16/32-bit Timer2 operation
        T3CON = 0x00;               //Stops any 16-bit Timer3 operation
        TMR3 = 0x00;                //Clear contents of the timer3 register
        TMR2 = 0x00;                //Clear contents of the timer2 register
        T2CONbits.TCKPS = 0b11;     //Set 256 prescaler
        
        //set Timer
        if (gameDiff == 1){
            PR3 = 0x0001;       //set period //0x0008
            PR2 = 0x65E1;       //0x8FD5
        }
        else if (gameDiff == 2){
            PR3 = 0x0001;       //set period    //0x000D
            PR2 = 0xFFFE;
        }
        else if (gameDiff == 3){
            PR3 = 0x0002;       //set period    //0x001B
            PR2 = 0x65E1;
        }
        
        //maxTime = ((PR3 << 16) & 0xFFFF0000) | (PR2 & 0x0000FFFF);    //concat PR3 and PR2
        maxTime = ((PR3 << 8) & 0xFF00) | ((PR2 >> 8) & 0x00FF);
        //        maxTime = maxTime << 16;
        //        maxTime = maxTime & 0xFFFF0000;
        //        maxTime = maxTime | (PR2 & 0x0000FFFF);
        //        //
        //        maxTime = maxTime >> 16;
        //        char maxTim[16] = "";
        //        sprintf(maxTim, "Time: %u", maxTime);
        //        lcd_clear();
        //        lcd_printstring(maxTim);
        //        __delay_ms(2000);
        //        lcd_clear();
        
        IPC2bits.T3IP = 0x7;         //Set up Timer3 interrupt for desired priority level
        IFS0bits.T3IF = 0;          //Clear the Timer3 interrupt status flag
        IEC0bits.T3IE = 1;          //Enable Timer3 interrupts
        T2CONbits.T32 = 1;          //Enable 32-bit Timer operation
        T2CONbits.TON = 1;          //turn on Timer2
        
        T4CON = 0x00;               //Stops any 16/32-bit Timer4 operation
        TMR4 = 0x00;                //Clear contents of the timer4 register
        PR4 = 0xFFFF;               //Load Timer4 with full 65536 ticks
        T4CONbits.TCKPS = 0b00;     //Set 1:1 prescaler
        
        IPC6bits.T4IP = 0x5;         //Set up Timer4 interrupt for desired priority level
        IFS1bits.T4IF = 0;          //Clear the Timer4 interrupt status flag
        IEC1bits.T4IE = 1;          //Enable Timer4 interrupts
        T4CONbits.TON = 1;          //turn on Timer4
        
        config_OC2();
        
        addToDuty = 1000;
        
        OC2RS = 0xFFFF;               //set period
        OC2R = (int)OC2RS/2;          //set duty
        
        while(stateSel == 4){
            
            opA = (rand() % 19) - 9; //to get [-9, 9]: generate a random int from [0, 18] then subtract 9
            opB = (rand() % 19) - 9;
            
            //EASY GAME
            if (gameDiff == 1){
                answer = opA + opB;
                pointNum = 1;
                sprintf(quesStr, "Q: %d + %d", opA, opB);
            }
            
            //AVE GAME
            if (gameDiff == 2){
                opNum = rand() % 2;
                if (opNum == 0){
                    answer = opA + opB;
                    pointNum = 1;
                    sprintf(quesStr, "Q: %d + %d", opA, opB);
                }
                else if (opNum == 1){
                    answer = opA - opB;
                    pointNum = 2;
                    sprintf(quesStr, "Q: %d - %d", opA, opB);
                }
            }
            
            //HARD GAME
            if (gameDiff == 3){
                opNum = rand() % 3;
                if (opNum == 0){
                    answer = opA + opB;
                    pointNum = 1;
                    sprintf(quesStr, "Q: %d + %d", opA, opB);
                }
                else if (opNum == 1){
                    answer = opA - opB;
                    pointNum = 2;
                    sprintf(quesStr, "Q: %d - %d", opA, opB);
                }
                else if (opNum == 2){
                    answer = opA * opB;
                    pointNum = 3;
                    sprintf(quesStr, "Q: %d * %d", opA, opB);
                }
            }
            
            lcd_printstring(quesStr);
            lcd_moveline(2);
            
            numans_i = 0;
            enterAns = 0;
            keypad_numpressed = -1;
            keypad_pressflag = 0;
            
            //Game Mode 1
            if (gameMode == 1){
                lcd_printstring("A: ");
                while(enterAns == 0){
                    while(!keypad_pressflag){
                        keypad_cycle();
                        if((stateSel != 4)|| enterAns)
                            break;
                    }
                    keypad_pressflag = 0;
                    if((keypad_numpressed != -1) && (keypad_numpressed != 11)){
                        if((keypad_numpressed == -3) && (numans_i != 0))
                            continue;
                        numAns[numans_i] = keypad_numpressed + 48;
                        numans_i++;
                    } else if (keypad_numpressed == 11){
                        numAns[(numans_i)-1] = 32;
                        numans_i--;
                    }
                    lcd_movecursor(2, 3);
                    lcd_printstring(numAns);
                    lcd_movecursor(2, numans_i + 3);
                    keypad_numpressed = -1;
                    __delay_ms(50);
                    if((numans_i >= 5) || enterAns || (stateSel != 4))
                        break;
                }
                
                
                userAns = atoi(numAns);
                lcd_clear();
                sprintf(ansStr, "User answer: %d", userAns);
                lcd_printstring(ansStr);
                if((userAns == answer) && (enterAns == 1))
                    playerScore = playerScore + pointNum;
                
                enterAns = 0;
                numans_i = 0;       //clear numAns index
                strcpy(numAns,"     "); //clear numAns string
            }
            //GAME MODE 2
            else if (gameMode == 2){
                numans_i = rand() % 3;
                
                if (numans_i == 0){
                    letterAns = 1;
                    sprintf(ansStr, "a:%d", answer);
                    lcd_movecursor(2,0);
                    lcd_printstring(ansStr);
                    sprintf(ansStr, "b:%d", answer + (rand()%10) + 1);
                    lcd_movecursor(2,5);
                    lcd_printstring(ansStr);
                    sprintf(ansStr, "c:%d", (rand()%10) - answer + 1);
                    lcd_movecursor(2,11);
                    lcd_printstring(ansStr);
                    
                }
                else if (numans_i == 1){
                    //sprintf(ansStr, "a:%d  b:%d  c:%d ", answer + (rand()%10) + 1, answer, (rand()%10) - answer);
                    letterAns = 2;
                    sprintf(ansStr, "a:%d", answer + (rand()%10) + 1);
                    lcd_movecursor(2,0);
                    lcd_printstring(ansStr);
                    sprintf(ansStr, "b:%d", answer);
                    lcd_movecursor(2,5);
                    lcd_printstring(ansStr);
                    sprintf(ansStr, "c:%d", (rand()%10) - answer + 1);
                    lcd_movecursor(2,11);
                    lcd_printstring(ansStr);
                }
                else if (numans_i == 2){
                    letterAns = 3;
                    sprintf(ansStr, "a:%d", (rand()%10) - answer + 1);
                    lcd_movecursor(2,0);
                    lcd_printstring(ansStr);
                    sprintf(ansStr, "b:%d", answer + (rand()%10) + 1);
                    lcd_movecursor(2,5);
                    lcd_printstring(ansStr);
                    sprintf(ansStr, "c:%d", answer);
                    lcd_movecursor(2,11);
                    lcd_printstring(ansStr);
                }
                
                float buffletterAns = 0;
                
                while(!enterAns){
                    buffletterAns = 3*((float)adcvalue/1023);
                    if (buffletterAns < 1){
                        userAns = 1;
                        lcd_movecursor(2, 0);
                    }
                    else if (buffletterAns < 2){
                        userAns = 2;
                        lcd_movecursor(2, 5);
                    }
                    else if (buffletterAns < 3){
                        userAns = 3;
                        lcd_movecursor(2, 11);
                    }
                    if(stateSel != 4){
                        break;
                    }
                }
                
                
                if((userAns == letterAns) && (letterAns != 0) && (enterAns == 1))
                    playerScore = playerScore + pointNum;
                
                enterAns = 0;
                pointNum = 0;
                letterAns = 0;
                userAns = 0;
                
            }
            
            lcd_clear();
            
        }
        
        
        lcd_clear();
        T2CONbits.TON = 0;          //turn off Timer2
        OC2RS = 0;                   //set period
        OC2R = 0;          //set duty
        stateSel = 5;
        goto stateGameOver;
        
    }
    
    //STATE 5: GAME OVER: GAME OVER!; Score: __(playerScore)__;
stateGameOver:
    {
        char scoreStr[16];
        sprintf(scoreStr, "   Score: %u     ", playerScore);
        LATAbits.LATA0 = 0;     //turn on LED
        lcd_clear();
        lcd_printstring("   GAME OVER!");
        lcd_moveline(2);
        lcd_printstring(scoreStr);
        
        __delay_ms(5000);
        stateSel = 1;
        T4CONbits.TON = 0;          //turn off Timer4
        goto stateStart;
        
    }
    
    return 0;
}

void __attribute__ ((interrupt)) _CNInterrupt(void) {
    int debounceCounter = 0;
    
    if (!PORTAbits.RA2){
        while ((!PORTAbits.RA2) && (debounceCounter < DEBOUNCEMAX)){
            debounceCounter++;
        }
        
        if((debounceCounter == DEBOUNCEMAX) && (stateSel != 4) && (stateSel != 2)){
            stateSel++;
        }
        else if((debounceCounter == DEBOUNCEMAX) && (stateSel == 4)){
            enterAns = 1;
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

void __attribute__ ((interrupt, no_auto_psv)) _ADC1Interrupt (void) {
    IEC0bits.AD1IE = 0; //disable interrupt
    IFS0bits.AD1IF = 0; //clear flag
    
    //copy ADC output to adcvalue
    adcvalue = ADC1BUF0;
    
    IEC0bits.AD1IE = 1; //enable interrupt
    IFS0bits.AD1IF = 0; //clear flag
    
    
    
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void) {
    stateSel = 5;
    IFS0bits.T3IF = 0; //clear timer3 flag
}

void __attribute__((interrupt, no_auto_psv)) _T4Interrupt(void) {
    IFS1bits.T4IF = 0;
    //timerPassed = ((TMR3 << 16) & 0xFFFF0000)|(TMR2 & 0x0000FFFF);
    timerPassed = ((TMR3 << 8) & 0xFF00) | ((TMR2 >> 8) & 0x00FF);
    //    timerPassed = timerPassed << 16;
    //    timerPassed = timerPassed & 0xFFFF0000;
    //    timerPassed = timerPassed | (TMR2 & 0x0000FFFF);
    //    if (timerPassed <= 1000){
    //        addToDuty = 1000;
    //    }
    //addToDuty = 1000 + (int)(9000*((float)(timerPassed)/maxTime));
    //addToDuty = addToDuty*((float)(timerPassed)/maxTime);
    
    if (stateSel == 4){
        if(dutyflag == 0){
            if ((OC2R + addToDuty) <= 62000)
                OC2R = OC2R + addToDuty;
            else{
                OC2R = 62000;
                dutyflag = 1;
                //addToDuty = 1000 + (int)(9000*((float)(timerPassed)/maxTime));
                //addToDuty = 1000;
            }
        }
        else if (dutyflag == 1) {
            if((OC2R - addToDuty) >= 1000)
                OC2R = OC2R - addToDuty;
            else {
                OC2R = 1000;
                dutyflag = 0;
                //addToDuty = 1000;
                addToDuty = 1000 + (int)(9000*((float)(timerPassed)/maxTime));
            }
        }
    }
    
}

void config_OC2(void){
    TRISAbits.TRISA0 = 0; //set RA0 is output
    
    __builtin_write_OSCCONL((OSCCON) | 0xBF); // clears IOLOCK bit
    _RP5R = 19; //assign OC1 to RP5
    __builtin_write_OSCCONL((OSCCON) | 0x40); // sets IOLOCK bit
    
    OC2CON1 = 0;
    OC2CON2 = 0;
    
    OC2CON2bits.SYNCSEL=0x1F;    //choose Timer4
    OC2CON2bits.OCTRIG = 0;
    
    OC2RS = 0xFFFF;                   //set period
    OC2R = (int)OC2RS/2;          //set duty
    
    OC2CON1bits.OCTSEL = 0b010;     //choose Timer4
    OC2CON1bits.OCM = 0b110;            //edge aligned PWM
}
