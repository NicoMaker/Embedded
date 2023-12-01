/*
 * File:   newmain.c
 * Author: Utente
 *
 * Created on 6 novembre 2023, 10.41
 */


#include <xc.h>
#define _XTAL_FREQ 8000000
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON       // RA5/MCLR pin function select (RA5/MCLR pin function is MCLR)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOD Reset enabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (RB4/PGM pin has digital I/O function, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection off)
#pragma config CP = OFF  // Code Protection bits (Program memory code protection off)7
#define XTAL_FREQ 8000000

void Bottone_premuto(void);

const int tempi_timer[4] = {
    500, 1000, 2000, 4000
};

int conta_interrupt, index;
char acceso_spento, oldread;

void main(void) {

    TRISB = 0x00;
    TRISA = 0x1E;
    INTCON = 0xA0;
    OPTION_REG = 0x02;

    TMR0 = 6;

    while (1)
        Bottone_premuto();
}

void __interrupt() ISR() {

    if (T0IF) {
        T0IF = 0;
        TMR0 = 6;

        conta_interrupt++;

        if (conta_interrupt >= tempi_timer[index]) {
            acceso_spento = !acceso_spento;

            if (acceso_spento)
                PORTB = 0x01;
            else
                PORTB = 0x00;

            conta_interrupt = 0;
        }
    }

}

void Bottone_premuto() {
    // non uso variabile read per non incrementare lo stack di memoria
    if (~PORTA & 0x02 && !oldread) { //controllo botone rimane premuto
        __delay_ms(10);

        if (~PORTA & 0x02 && !oldread) {
            index++;
            conta_interrupt = 0;

            if (index >= 4)
                index = 0;
        }

    }

    oldread = ~PORTA & 0x02; // in questo modoil valore che del bottone lo asscocio alla variabile oldread
}