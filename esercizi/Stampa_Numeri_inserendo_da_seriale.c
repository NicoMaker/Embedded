/*
 * File:   newmain.c
 * Author: nicol
 *
 * Created on 5 dicembre 2023, 10.53
 */


#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config WRT = OFF 
#pragma config CP = OFF  

#define _XTAL_FREQ 8000000
#include <xc.h>

#define L_ON 0x0F
#define L_OFF 0x08
#define L_CLR 0x01
#define L_L1 0x80
#define L_L2 0xC0
#define L_CR 0x0F
#define L_NCR 0x0C
#define L_CFG 0x38
#define L_CUR 0x0E

#define COMMAND 0
#define DATA 1

#define LCD_PORT PORTD
#define LCD_TRIS TRISD

#define LCD_EN PORTEbits.RE1
#define LCD_RS PORTEbits.RE2

void initLCD(void);
void initUart(long int); 
void sendLCD(char,char);
char SumNumberFromString(char *);
void printStringLCD(char *);

char string [10];
char index, endSend, result;

void main(void) {
    initLCD();
    initUart(9600);
    
    while(1)
    {
        if(endSend)
        {
            result = SumNumberFromString(string);
//            result = SumNumberFromString(string);
//            printStringLCD(string);
            printStringLCD(string);
            
            
            endSend = 0;
            index = 0;
        }    
    }    
}

void initLCD() {
    LCD_TRIS = 0x00;
    TRISE &= ~0x06;

    LCD_EN = 0;
    LCD_RS = 0;

    __delay_ms(20);
    LCD_EN = 1;
    sendLCD(L_CFG, COMMAND);

    __delay_ms(5);
    sendLCD(L_CFG, COMMAND);

    __delay_ms(1);
    sendLCD(L_CFG, COMMAND);

    sendLCD(L_OFF, COMMAND);
    sendLCD(L_ON, COMMAND);
    sendLCD(L_CLR, COMMAND);
    sendLCD(L_CUR, COMMAND);
    sendLCD(L_L1, COMMAND);
}
void initUart(long int baudRate) {
    //TRISC = 0x80;
    TRISC |= 0x80; //RC7 forzato a 1
    TRISC &= ~0x40; //RC6 forzato a 0

    TXSTA |= 0x20;
    RCSTA |= 0x90;
    INTCON |= 0xC0;
    PIE1 |= 0x20;

    //(long) ? il cast a long di 64UL*baudRate
    SPBRG = (_XTAL_FREQ / (long) (64UL * baudRate)) - 1;
}

void sendLCD(char dato, char rs) {
    LCD_EN = 1;
    LCD_PORT = dato;
    LCD_RS = rs;
    __delay_ms(3);

    LCD_EN = 0;
    __delay_ms(3);

    LCD_EN = 1;
}

char SumNumberFromString(char *calc)
{
    char i = 0, sum = 0;
    
    while(calc[i] != '\0')
    {
        for(char j = 0; j < 10; j++)
        {
          if(calc[i] == (j+48))
              sum += j;
        }
        i++;
    } 
    
    return sum;
}

void printStringLCD(char *s) {
    char i = 0;
    while (s[i] != '\0') {
        sendLCD(s[i++], DATA);
    }
    
    sendLCD('=', DATA);
    sendLCD('0'+result , DATA);
}

void __interrupt() ISR() {
    if (RCIF) {
        string[index] = RCREG; // RECREG -> leggge ogni carattere e lo manda 1 alla volta
        // 13 � CR tabella ascii
        if (string[index] == 13) {
            string[index] = '\0';
            endSend = 1;
        }
        index++;
        RCIF = 0;
        //PORTD = RCREG;
    }
}