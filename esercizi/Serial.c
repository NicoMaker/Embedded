#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = ON
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF


#include <xc.h>
#include <pic16f877a.h>
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

#define LCD_EN PORTEbits.RE1
#define LCD_RS PORTEbits.RE2


void initUart(long int);
void sendUARTString(char *);
void initLCD(void);
void sendLCD(char, char);
void printStringLCD(char *s);

char string[10];
char index;
char endSend;

void main(void) {
    initUart(9600);
    initLCD();

    TRISD = 0x00;
    INTCON |= 0xC0;

    PIE1 |= 0x20;

    while (1) {
        if (endSend) {
            sendLCD(0x01, COMMAND);
            endSend = 0;
            index = 0;
            printStringLCD(string);
        }
        //sendUARTString("ciao");
        //TXREG = 'c';
        //__delay_ms(1000);
    }
}

void initLCD() {
    LCD_TRIS = 0x00;
    TRISE &= ~0x06;

    LCD_EN = 0;
    LCD_RS = 0;

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

void initUart(long int baudRate) {
    //TRISC = 0x80;
    TRISC |= 0x80; //RC7 forzato a 1
    TRISC &= ~0x40; //RC6 forzato a 0

    TXSTA |= 0x20;
    RCSTA |= 0x90;

    //(long) è il cast a long di 64UL*baudRate
    SPBRG = (_XTAL_FREQ / (long) (64UL * baudRate)) - 1;
}

void sendUARTString(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        while (!PIR1bits.TXIF);
        TXREG = str[i++];
    }
}

void printStringLCD(char *s) {
    char i = 0;
    while (s[i] != '\0') {
        sendLCD(s[i++], DATA);
    }
}

void __interrupt() ISR() {
    if (RCIF) {
        string[index] = RCREG;
        if (string[index] == 13) {
            string[index] = '\0';
            endSend = 1;
        }
        index++;
        RCIF = 0;
        //PORTD = RCREG;
    }
}