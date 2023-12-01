/*
 * File:   newmain.c
 * Author: Utente
 *
 * Created on 10 novembre 2023, 9.12
 */

// PIC 16F877A -> MODELLO PIC GENIUS


#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config WRT = OFF 
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off


#include <xc.h>

#define L_ON 0x0F
#define L_OFF 0x08
#define L_CLR 0x01
#define L_L1 0x80
#define L_L2 0xC0
#define L_CR 0x0F
#define L_NCR 0x0C
#define L_CFG 0x38
#define L_CUR 0x0E


#define _XTAL_FREQ 8000000
#define COMMAND 0
#define DATA 1
#define LCDPORT PORTD
#define LCDEN PORTEbits.RE1 
#define LCDRS PORTEbits.RE2

//#define SETRD PORTD != 0X01;
//PORTD &=  ~0x01; -> PORTDbits.RD1 = 0;
//PORTD != 0X01;   ->   PORTDbits.RD0 = 1;


void SendLCD(char, char); // 1 parametro -> dato , 2 paramtro -> comando
void InitLCD(void);
void PrintLCD(char *);

char frase[11] = "Continuiamo";

void main(void) {

    InitLCD();
    //    SendLCD('P',DATA);
    //    SendLCD('a',DATA);
    //    SendLCD('u',DATA);
    //    SendLCD('u',DATA);
    //    SendLCD('a',DATA);
    
    
    SendLCD(L_L1+3,COMMAND);
    PrintLCD("Ciao a tutti");
    
    SendLCD(L_L2+3,COMMAND);
    PrintLCD("Seconda riga");

//    PrintLCD(frase);

    while (1) {
    }
    return;
}

void PrintLCD(char s []) {

    int i = 0;
    while(s[i] != '\0')
        SendLCD(s[i++],DATA);
}

void InitLCD() {
    TRISD = 0x00;
    TRISE &= ~0x06; // forzo a 0 bit 1 e 2, altri invece a 1

    LCDRS = 0;
    LCDEN = 0;
    __delay_ms(20);
    LCDEN = 1;

    SendLCD(L_CFG, COMMAND);
    __delay_ms(5);
    SendLCD(L_CFG, COMMAND);
    __delay_ms(1);
    SendLCD(L_CFG, COMMAND);

    SendLCD(L_OFF, COMMAND);
    SendLCD(L_ON, COMMAND);
    SendLCD(L_CLR, COMMAND);
    SendLCD(L_CUR, COMMAND); // 0x0E cursor no, no blinkk
    SendLCD(L_L1, COMMAND);
}

void SendLCD(char dato, char rs) {
    LCDEN = 1;
    LCDPORT = dato;
    LCDRS = rs;

    __delay_ms(3);
    LCDEN = 0; //  comunicaziione parallela a 8 bit

    __delay_ms(3);
    LCDEN = 1;

}