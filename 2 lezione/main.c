/*
 * File:   main.c
 * Author: Utente
 *
 * Created on 18 ottobre 2023, 14.57
 */

#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON       // RA5/MCLR pin function select (RA5/MCLR pin function is MCLR)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOD Reset enabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (RB4/PGM pin has digital I/O function, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection off)
#pragma config CP = OFF 


#include <xc.h>
#define _XTAL_FREQ 8000000

char d[10] =
{
    0xEE,0x28,0xCD, 0x6D, 0x2B, 0x67, 0xE7, 0x2C, 0xEF, 0x6F
};

void main(void) {
        
    TRISB = 0x00;
    INTCON = 0xA0; // 0b10100000; 
    OPTION_REG = 0b00000111; //0x07
    
    while(1){
        for(int i = 0; i < 10; i++)
        {
            PORTB = d[i];
            __delay_ms(1000);
        }
    }
    
}

void __interrupt() ISR() //INTERRUPT SERVICE ROUTINE
{
    if(T0IF) // T0IF == 1
    {
        T0IF = 0;
    }
}