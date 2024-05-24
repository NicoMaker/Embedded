#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = ON
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 8000000
#include <xc.h>

void main(void) {

    // imposto i tris
    TRISB = 0x00;
    TRISD = 0x00;
    // TRISA = 0x01;
    TRISA = 0xFF;
    TRISC = 0x00;

    // imposto adcon
    // ADCON0 = 0x01;
    // ADCON0 = 0x09;
    ADCON0 = 0x11;
    // ADCON1 = 0x8E
    ADCON1 = 0x80;
    PORTCbits.RC5 = 1;
    __delay_us(20);

    while (1) {
        // ADCON0 |= 0x04;
        ADCON0bits.GO = 1;

        while (ADCON0bits.GO != 0);

        PORTB = ADRESL;
        PORTD = ADRESH;

        int temperature = ADRESL + (ADRESH << 8);

        if (temperature > 80)
            PORTCbits.RC2 = 1;
        else
            PORTCbits.RC2 = 0;

        __delay_ms(20);
    }
}