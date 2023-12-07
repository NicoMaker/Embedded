/*
 * File:   newmain.c
 * Author: nicol
 *
 * Created on 7 dicembre 2023, 14.09
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

char BottoneCliccato();
void initUART(long int);
void SendUARTString(char *);

const char c [3] = {
    0xFE, 0xFD, 0xFB
};

void main(void) {

    initUART(9600);
    while (1) {
        char letturabottone = BottoneCliccato();

        if (letturabottone == 6)
            SendUARTString("five");
    }
}

char BottoneCliccato() {
    char k = 99;


    TRISD = 0x0F;
    TRISB &= ~0x07;

    for (char i = 0; i < 3; i++) {
        PORTB = c[i];
        __delay_us(50);

        char read = ~PORTD & TRISD;

        for (char j = 0; j < 4; j++) {
            if (read & (1 << j))
                k = j + (4 * i);
        }
    }

    TRISD = 0x00;
    __delay_us(20);

    return k;
}

void initUART(long int baudRate) {
    INTCON |= 0xC0;
    PIE1 |= 0X20;


    TRISCbits.TRISC7 = 1; // impongo input RX
    TRISCbits.TRISC6 = 0; // impongo output TX

    TXSTAbits.TXEN = 1; //abilito sottomodulo di TX
    RCSTAbits.SPEN = 1; // abilito il modulo UART
    RCSTA |= 0x90;
    SPBRG = (_XTAL_FREQ / (long) (64UL * baudRate)) - 1; // uso variabile del calcolo senza segno UL
}

void SendUARTString(char *str) {
    int i = 0;

    while (str[i] != '\0') {

        while (!PIR1bits.TXIF); // TXIF -> 1 trasmissione 0 -> non trasmetto
        TXREG = str[i]; // stampo un carattere alla volta tutte la stringa
        i++;
    }
}