#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = ON
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define L_ON 0x0F
#define L_OFF 0x08
#define L_CLR 0x01
#define L_L1 0x80
#define L_L2 0xC0
#define L_CR 0x0F
#define L_NCR 0x0C
#define L_CFG 0x38
#define L_CUR 0x0E
#define LCD_EN PORTEbits.RE1
#define LCD_RS PORTEbits.RE2

#define COMMAND 0
#define DATA 1

#define LCD_PORT PORTD
#define LCD_TRIS TRISD
#define HEATER PORTCbits.RC5
#define CULER PORTCbits.RC2

#include <xc.h>
#define _XTAL_FREQ 8000000

char bottone_premuto, set_point = 45;
char string_temp [16];
int temperature;

void initLCD(void);
void sendLCD(char, char);
void printStringLCD(char *);
void adcInit(void);
void convert(int, char*);
char NumberLenght(long);
int readADC(void);
void initPicConfig(void);

void main(void) {

    TRISB = 0x00;
    TRISD = 0x00;
    TRISB = 0x18; // impposto come input il bottone 3 e 4 

    initPicConfig();
    initLCD();
    adcInit();

    convert(set_point, string_temp);
    printStringLCD(string_temp);

    while (1) {
        temperature = readADC();
        if (temperature > 125) {
            HEATER = 0;
            CULER = 1;
        } else {
            HEATER = 1;
            CULER = 0;
        }
        __delay_ms(5);

        //if (~PORTB & 0x08) {
        //}
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
    char i = 1;
    while (s[i] != '\0') {
        sendLCD(s[i++], DATA);
    }
    bottone_premuto = 1;
}

void adcInit() {
    //Almeno un ingresso in input
    TRISA = 0xFF; //check later for interrupt issue...

    //Imposto RC5 come output per mettere in output l'heater
    TRISC |= 0x20;

    //CH0, CH1, CH2 impostati a zero per selezionare il primo canale
    ADCON0 |= 0x11;
    ADCON1 |= 0x80;

    //Per vedere uqali bit sono associati a quali componenti della macchina, guarda i pulsantini bianchi sul simulatore (non serve vedere schema elettrico)
    //Accendi heater
    PORTC |= 0x20;

    __delay_us(20);
}

void convert(int num, char* s) {
    char numLenght = NumberLenght(num);
    for (char i = 0, j = numLenght - 1; i < numLenght; i++, j--) {
        long divisore = 1;
        for (char k = j; k > 0; k--) {
            divisore *= 10;
        }

        char count = 0;
        while (num - divisore >= 0) {
            num -= divisore;
            count++;
        }
        s[i] = '0' + count;
        s[i + 1] = '\0';

    }
}

char NumberLenght(long num) {
    char numLength = 1;
    long baseTester = 9;
    while (num - baseTester >= 0) {
        //Sottraggo al numero un valore sempre pi� grande per calcolare la lunghezza del numero di input.
        baseTester = baseTester * 10 + 9;
        numLength++;
    }
    return numLength;
}

int readADC() {
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO != 0);
    return ADRESL + (ADRESH << 8);
}

void initPicConfig() {
    TRISA = 0xFF; // imposto tutto input
    TRISB = 0x03; // RB0 e RB1 come input, il resto output
    TRISC = 0x00; // tutti output
    TRISD = 0x00; // pilotaggio LCD Output
    TRISE = 0x00; // pilotaggio LCD output
}