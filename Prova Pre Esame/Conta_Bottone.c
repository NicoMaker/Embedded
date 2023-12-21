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
void sendStringLCD(char *);

char buttonNames[3][3] = {
    "RB3",
    "RB4",
    "RB5"
};

void main(void) {
    initLCD();

    TRISB |= 0x38;

    //sendStringLCD("ciao");
    while (1) {
        for(char button = 0; button < 3; button++){
            if(~PORTB & (00001000 << (button))){
                sendStringLCD(buttonNames[button]);
            }
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

void sendStringLCD(char * s) {
    char i = 0;
    while (s[i] != '\0') {
        sendLCD(s[i++], DATA);
    }
}
