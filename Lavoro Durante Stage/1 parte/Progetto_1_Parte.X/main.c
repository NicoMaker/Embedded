/*
 * Marano Nicola
 */


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

//PORTD &= ~0x01   =>  PORTDbits.RD0 = 0
//PORTD |= 0x01    =>  PORTDbits.RD0 = 1
#define LCD_EN PORTEbits.RE1
#define LCD_RS PORTEbits.RE2

void initLCD(void);
void sendLCD(char, char);
void printStringLCD(char *);
char buttonMatrix(void);

char x, bottone_premuto;

const char columnConfig[3] = {
    //Configurazione delle colonne con uno basso e resto alto
    0xFE,
    0xFD,
    0xFB
};

void main(void) {
    initLCD();

    printStringLCD("ciao");
    sendLCD(L_L2, COMMAND);

    ADCON1 = 0x06;
    TRISA = 0x20;

    while (1) {
        x = buttonMatrix();
        if (x == 99) {
            bottone_premuto = 0;
            continue;
        }
        TRISD = 0x00;
        if (x == 55)
            continue;

        switch (x) {
            case 3:
                printStringLCD("1");
                break;
            case 7:
                printStringLCD("2");
                break;
            case 11:
                printStringLCD("3");
                break;
            case 2:
                printStringLCD("4");
                break;
            case 6:
                printStringLCD("5");
                break;
            case 10:
                printStringLCD("6");
                break;
            case 1:
                printStringLCD("7");
                break;
            case 5:
                printStringLCD("8");
                break;
            case 9:
                printStringLCD("9");
                break;
            case 0:
                printStringLCD("*");
                break;
            case 4:
                printStringLCD("0");
                break;
            case 8:
                printStringLCD("#");
                break;
        }
    }
}

void initLCD() {
    LCD_TRIS = 0x00;
    TRISE &= ~0x06; //Rs ed EN a 0

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

void sendLCD(char dato, char rs) {
    LCD_EN = 1;
    LCD_PORT = dato;
    LCD_RS = rs;
    __delay_ms(3);

    LCD_EN = 0;
    __delay_ms(3);

    LCD_EN = 1;
}

void printStringLCD(char *s) {
    if (bottone_premuto)
        return;
    char i = 0;
    while (s[i] != '\0') {
        sendLCD(s[i++], DATA);
    }
    bottone_premuto = 1;
}

char buttonMatrix() {
    char k = 99;
    TRISB = 0x3F;

    if (~PORTB & TRISB) {
        __delay_ms(2);

        TRISD = 0x00;
        printStringLCD("TRISB");
        return 55;
    }
    if (~PORTA & TRISA) {
        __delay_ms(2);

        TRISD = 0x00;
        printStringLCD("TRISA");
        return 55;
    }
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
    return k;
}