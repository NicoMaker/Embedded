#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON       // RA5/MCLR pin function select (RA5/MCLR pin function is MCLR)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOD Reset enabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (RB4/PGM pin has digital I/O function, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection off)
#pragma config CP = OFF         // Code Protection bits (Program memory code protection off)

#include <xc.h>
#define true 1
#define false 0
#define _XTAL_FREQ 8000000

void main(void) 
{
    TRISB = 0x00;
    TRISA = 0x1e;
    
    
    //PORTB = 0xff; tutti accessi
    //PORTB = 0xaa; accessi alternati diversi        
    //PORTB = 0x55; //accesi alternati dal primo
    
    //while(true)
    //{
    //    PORTB = 0xff;
    //    __delay_ms(500); //ritardo 
    //    PORTB = 0x00;
    //    __delay_ms(500);
    //}
    
    while(true)
    {
        PORTB = PORTA >> 1;
    } 
}
