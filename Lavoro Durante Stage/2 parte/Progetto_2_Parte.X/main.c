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
void initUART(long int);
void SendUARTString(char *);

char x, bottone_premuto, index, endsend;
char string[10];

const char columnConfig[3] = {
    //Configurazione delle colonne con uno basso e resto alto
    0xFE,
    0xFD,
    0xFB
};

void main(void) {
    initLCD();
    initUART(9600);

    printStringLCD("ciao");
    sendLCD(L_L2, COMMAND);

    ADCON1 = 0x06;
    TRISA = 0x20;

    while (1) {
        TRISD = 0x00;
        if (endsend) {
            endsend = 0;
            index = 0;
            printStringLCD(string);
        }
        x = buttonMatrix();
        if (x == 99) {
            bottone_premuto = 0;
            continue;
        }
        if (x == 55)
            continue;

        switch (x) {
            case 3:
                SendUARTString("1");
                break;
            case 7:
                SendUARTString("2");
                break;
            case 11:
                SendUARTString("3");
                break;
            case 2:
                SendUARTString("4");
                break;
            case 6:
                SendUARTString("5");
                break;
            case 10:
                SendUARTString("6");
                break;
            case 1:
                SendUARTString("7");
                break;
            case 5:
                SendUARTString("8");
                break;
            case 9:
                SendUARTString("9");
                break;
            case 0:
                SendUARTString("*");
                break;
            case 4:
                SendUARTString("0");
                break;
            case 8:
                SendUARTString("#");
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
        SendUARTString("TRISB");
        return 55;
    }
    if (~PORTA & TRISA) {
        __delay_ms(2);

        TRISD = 0x00;
        SendUARTString("TRISA");
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

void initUART(long int baudRate) {
    TRISCbits.TRISC7 = 1; // impongo input RX
    TRISCbits.TRISC6 = 0; // impongo output TX

    TXSTAbits.TXEN = 1; //abilito sottomodulo di TX
    RCSTAbits.SPEN = 1; // abilito il modulo UART
    RCSTAbits.CREN = 1; // abilito il ricevimento continuo dei dati
    SPBRG = (_XTAL_FREQ / (long) (64UL * baudRate)) - 1; // uso variabile del calcolo senza segno UL

    INTCONbits.GIE = 1; // abilito gli interrupt
    INTCONbits.PEIE = 1; // abilito gli interrupt periferichi
    PIE1bits.RCIE = 1; // abilito gli interrupt dlla trasmisione seriale
}

void SendUARTString(char *str) {
    if (bottone_premuto)
        return;
    int i = 0;

    while (str[i] != '\0') {

        while (!PIR1bits.TXIF); // TXIF -> 1 trasmissione 0 -> non trasmetto
        TXREG = str[i]; // stampo un carattere alla volta tutte la stringa
        i++;
    }
    bottone_premuto = 1;

}

void __interrupt() ISR() {
    if (RCIF) {
        string[index] = RCREG;
        RCIF = 0; // importante se no se no si ricrea output

        if (string[index] == 13) {
            string[index] = '\0';
            endsend = 1;
        }
        index++;
    }
}