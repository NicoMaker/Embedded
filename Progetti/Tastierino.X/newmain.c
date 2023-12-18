#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = ON
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF


#include <xc.h>
#define _XTAL_FREQ 8000000

//display 7 segments
#define SEG_A 0x01
#define SEG_B 0x02
#define SEG_C 0x04
#define SEG_D 0x08
#define SEG_E 0x10
#define SEG_F 0x20
#define SEG_G 0x40
#define SEG_P 0x80

#define DISP1 0x04
#define DISP2 0x08
#define DISP3 0x10
#define DISP4 0x20

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
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
};

const char columnConfig[3] = {
    //Configurazione delle colonne con uno basso e resto alto
    0xFE,
    0xFD,
    0xFB
};


void initUart(long int);
char buttonMatrix(void);
void sendUARTString(char*);

char buttonIndex, endSend;

void main(void) {
    initUart(9600);

    while (1) {
        buttonIndex = buttonMatrix();

        switch (buttonIndex) {
            case 1:
                sendUARTString("seven");
                break;
            case 2:
                sendUARTString("four");
                break;
            case 3:
                sendUARTString("one");
                break;
            case 6:
                sendUARTString("five");
                break;
            default:
                endSend = 0;
        }
    }
}

void initUart(long int baudRate) {
    //TRISC = 0x80;
    TRISC |= 0x80; //RC7 forzato a 1
    TRISC &= ~0x40; //RC6 forzato a 0

    TXSTA |= 0x20;
    RCSTA |= 0x90;

    INTCON |= 0xC0;
    PIE1 |= 0x20;
    //(long) � il cast a long di 64UL*baudRate
    SPBRG = (_XTAL_FREQ / (long) (64UL * baudRate)) - 1;
}

char buttonMatrix() {
    char k = 99;

    TRISD = 0x0F;
    TRISB &= ~0x07;

    for (char col = 0; col < 3; col++) {

        PORTB = columnConfig[col];
        __delay_us(50);

        char read = ~PORTD & TRISD;

        for (char row = 0; row < 4; row++) {
            if (read & (1 << row)) {
                k = row + col * 4;
            }
        }
    }

    //TRISD = 0x00;
    //__delay_ms(20);

    return k;
}

void sendUARTString(char *str) {
    if (endSend) return;
    int i = 0;
    while (str[i] != '\0') {
        while (!PIR1bits.TXIF);
        TXREG = str[i++];
    }
    endSend = 1;
}
