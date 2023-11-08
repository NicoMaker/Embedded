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
#pragma config CP = OFF         // Code Protection bits (Program memory code protection off)

#define SEG_A 0x04
#define SEG_B 0x08
#define SEG_C 0x20
#define SEG_D 0x40
#define SEG_E 0x80
#define SEG_F 0x02
#define SEG_G 0x01
#define SEG_P 0x10

//Tutti i segmenti per ogni display
const char display[10] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    SEG_B | SEG_C,
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,
    SEG_B | SEG_C | SEG_F | SEG_G,
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G
};

const char tempi_timer[4] = {
    500, 1000, 2000, 4000
};

void Stampa_Numeri(char, char);
void Gestione_pulsanti(void);

int conta_interrupt, delay_time;
char unita, decine, cambio_dec_uni, index, oldread;

void main(void) {

    TRISB = 0x00;
    TRISA = 0x1E;
    INTCON = 0xa0; //registro bit che derivano da altri azioni -> quindi serve per l'interrupt
    OPTION_REG = 0x02; //perscaller

    TMR0 = 6;

    index = 0;
    delay_time = 500;

    while (1) {
        Stampa_Numeri(unita, decine);
    }
}

void __interrupt() ISR() {

    if (T0IF)
        T0IF = 0;

    TMR0 = 6;
    conta_interrupt++;

    cambio_dec_uni = !cambio_dec_uni;

    if (conta_interrupt > delay_time) {

        unita++;
        conta_interrupt = 0;
    }

    if (unita > 9) {
        decine++;
        unita = 0;
    }
}

// char -> 2 alla 8 numeri massimi

void Stampa_Numeri(char u, char d) {

    if (cambio_dec_uni)
        PORTB = display[u] | SEG_P;
    else
        PORTB = display[d];


    if (u == 9)
        d++;
}

void Gestione_pulsanti() {

    char read;

    read = ~PORTA & 0x02;

    if (read && oldread) {
        __delay_ms(5);

        read = ~PORTA & 0x02;

        if (read && oldread) {
            delay_time = tempi_timer[index];
            index++;
        }
    }
    
    oldread = read;
}