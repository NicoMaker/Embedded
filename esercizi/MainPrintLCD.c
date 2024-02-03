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

//Tali define derivano dalla tabella di classroom per configurare il compurtamento del display
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
//void printStringLCD(char *);
void extractDigitsNumber(long);
char NumberLenght(long);

//char number[3];
//char digitsNumber[2] = {};

void main(void) {
    initLCD();

    char str[10];
    
    //char unit = 255 / 100;
    //sendLCD('0' + unit, DATA);

    extractDigitsNumber(265719, str);
    printStringLCD(str);

    sendLCD(L_L2, COMMAND);

    extractDigitsNumber(2, str);

    while (1) {

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

void extractDigitsNumber(long num, char *s) {
    //Calcolare la lunghezza del numero in input
    char numLenght = NumberLenght(num);

    //Ciclo per tutta la lunghezza del numero
    //Parto dalla cifra di sinistra e arrivo alla cifra di destra
    for (char i = 0, j = numLenght - 1; i < numLenght; i++, j--) {
        long divisore = 1;
        //Preparo il divisore per estrarre la cifra più a sinistra del numero di input
        for (char k = j; k > 0; k--) {
            divisore *= 10;
        }

        char count = 0;
        //In base al valore della cifra, sottraggo il divisore al numero e incremento la conta
        //Tale conta è il valore della cifra
        while (num - divisore >= 0) {
            //Sottraggo al numero il divisore per rendere più facilil i conti nei cicli successivi.
            num -= divisore;
            count++;
        }
        //Invio la cifra al display che la posizionerà da sinistra a destra
        //L'ordine delle cifre viene così mantenuto.
        //sendLCD('0' + count, DATA);
        s[i] = '0' + count;
    }
}

char NumberLenght(long num) {
    char numLength = 1;
    long baseTester = 9;
    while (num - baseTester >= 0) {
        //Sottraggo al numero un valore sempre più grande per calcolare la lunghezza del numero di input.
        baseTester = baseTester * 10 + 9;
        numLength++;
    }
    return numLength;
}

void printStringLCD(char *s) {
    char i = 0;
    while (s[i] != '\0') {
        sendLCD(s[i++], DATA);
    }
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