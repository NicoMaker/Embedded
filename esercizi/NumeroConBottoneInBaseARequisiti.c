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
void initTmr0(void);
void sendLCD(char, char);
void printStringLCD(char *);

char numSerial[3];

int timer;
int countInterrupt;

char endSend;
char tmrInit;
char buttonIndex;

void main(void) {
    initLCD();

    TRISB = 0xFF;
    TRISD = 0x00;

    while (1) {
        if (buttonIndex > 3) {
            buttonIndex = 0;
        }
        //Alla pressione del pulsante RB3...
        if ((~PORTB & 0x08) && !tmrInit) {
            initTmr0();
        }

        if (timer) {
            //printStringLCD(numSerial);
            switch (buttonIndex - 1) {
                case 0:
                    printStringLCD(numSerial);
                    break;
                case 1:
                    PORTD = 0xFF;
                    break;
                case 2:
                    sendLCD(L_CLR, COMMAND);
                    timer = 0;
                    break;
            }
        }
    }
}

void initTmr0() {
    if (tmrInit) return;
    INTCON = 0xA0;
    OPTION_REG = 0x02;
    TMR0 = 6;
    buttonIndex++;
    tmrInit = 1;
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

void printStringLCD(char *s) {
    if (endSend) return;
    sendLCD(L_CLR, COMMAND);
    char i = 0;
    while (s[i] != '\0') {
        sendLCD(s[i++], DATA);
    }
    endSend = 1;
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

void __interrupt() ISR() {
    if (T0IF) {
        T0IF = 0;
        TMR0 = 6;
        countInterrupt++;
        if (countInterrupt > 1000) {
            countInterrupt = 0;
            endSend = 0;
            timer++;
            tmrInit = 0;
            numSerial[0] = '0' + timer;
        }
    }
}

/*
 codice che alla pressione di un pulsante invia al terminale a al display LCD il nome dle pulsante;
 * codice che alla pressione di un tasto inizi un conteggio preogressivo che si vedrà su display e rseriale
 * Seconda pressione il timer si ferma
 * terza pressione il contatore si resetta.
 
 */