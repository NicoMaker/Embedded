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

const char ColomnConfig[3] = {
    0xFE,
    0xFD,
    0xFB
};


void INITUART(long int);
char BottonMatrix(void);
void SENDUARTSTRING(char *);

void main(void) {
    return;
}

void INITUART(long int baudRate) // trasmisione e ricezione seriale
{
    TRISCbits.TRISC7 = 1;
    TRISCbits.TRISC6 = 0;

    //    TRISC |= 0x80;
    //    TRISC = 0x00 | 0x80;
    //    TRISC &= ~0x40;

    TXSTAbits.TXEN = 1;
    // TXSTA |= 0x20; trasmetto trasmettittore 

    // RCSTAbits.CREN = 1;
    // RCSTAbits.SPEN = 1;

    RCSTA |= 0x90; // trasmetto ricevitore

    INTCON |= 0xC0;
    PIE1 |= 0x20;

    SPBRG = (_XTAL_FREQ / (long) (64UL * baudRate)) - 1; // uso variabile del calcolo senza segno UL
}

char BottonMatrix(){
    
    char k = 99;
    
    TRISD = 0x0F;
    TRISB &= ~0x07;

    for(char col = 0; col < 3; col++){

        PORTB = ColomnConfig[col];
        __delay_us(500);

        char read = ~PORTD & TRISD;

        for(char row = 0; row < 4; row++){
            if(read &(1 << row))
                k = row + (col * 4);
        }
    }
}

void SENDUARTSTRING(char * string) { // inviare i dati della stringa alla seriale 

    char i = 0;

    while (string[i] != '\0') {
        
        while (!PIR1bits.TXIF);
        TXREG = string[i++];
    }
}