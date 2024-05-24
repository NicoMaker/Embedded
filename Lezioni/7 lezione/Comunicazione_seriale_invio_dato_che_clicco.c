/*
 * File:   newmain.c
 * Author: nicol
 *
 * Created on 1 dicembre 2023, 9.16
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

void initUART(long int);

void main(void) {
    
    initUART(9600);
    TRISD = 0x00;
    
    while(1)
    {
        
    } 
    
    return;
}

void initUART(long int baudRate)
{
   TRISCbits.TRISC7 = 1; // impongo input RX
   TRISCbits.TRISC6 = 0;  // impongo output TX
   
   TXSTAbits.TXEN = 1; //abilito sottomodulo di TX
   RCSTAbits.SPEN = 1; // abilito il modulo UART
   RCSTAbits.CREN = 1; // abilito il ricevimento continuo dei dati
   SPBRG = (_XTAL_FREQ/(long)(64UL*baudRate))-1; // uso variabile del calcolo senza segno UL

   INTCONbits.GIE = 1; // abilito gli interrupt
   INTCONbits.PEIE = 1; // abilito gli interrupt periferichi
   PIE1bits.RCIE = 1; // abilito gli interrupt dlla trasmisione seriale
}

void __interrupt() ISR()
{
    if(RCIF)
    {
        TXREG = RCREG;
        RCIF = 0; // importante se no se no si ricrea output
    }

//    if(T0IF)
//    {
//        T0IF = 0;
//    }    
}