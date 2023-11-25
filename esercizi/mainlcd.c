/*
 * File:   mainlcd.c
 * Author: Utente
 *
 * Created on 17 novembre 2023, 9.40
 *  PIC
 */

#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Detect Enable bit (BOD enabled)
#pragma config LVP = ON        // Low-Voltage Programming Enable bit (RB4/PGM pin has digital I/O function, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Data memory code protection off)
#pragma config WRT = OFF
#pragma config CP = OFF


#include <xc.h>
#define _XTAL_FREQ 8000000
#define COMMAND 0
#define DATA 1
#define LCDPORT PORTD
#define LCDEN  PORTEbits.RE1 //più operazioni che scrivere tutta la porta. Comunque accede a tutti gli 8 bit
#define LCDRS  PORTEbits.RE2
#define L_ON 0x0F
#define L_OFF 0x08
#define L_CLR 0x01
#define L_L1 0x80
#define L_L2 0xc0
#define L_CR 0x0F
#define L_NCR 0x0c
#define L_CFG 0x38 
#define L_CUR 0x0E
#define D TRISD
#define E TRISE

//#define SETRD0 PORTD != 0x01

//PORTD &= ~0x01 -> PORTDbits.RD0 = 0; SETTO IL BIT A 0
//PORTD != 0x01 -> PORTDbits.RD0 = 1; SETTO IL BIT A 1

void sendLCD(char , char);
void initLCD();
void printLCD(char *);
void printLCDMod(char *);
//int pot(int, int);
void convert(int, char*);

char frase[16] = "Continuiamo";

void main(void) {
    initLCD();
    
    /*
     //sendLCD('p', DATA)
    sendLCD(L_L1 + 3, COMMAND);     //vai alla prima riga e inizi dalla quarta casella
    printLCD(frase);
    sendLCD(L_L2, COMMAND);     //vai a capo
    printLCD("Ciao a tutti");
     */
    
    
    char arrayDiChar[16];

    convert(-1788, arrayDiChar);

    printLCDMod(arrayDiChar);

    
    while(1)
    {
        
    }
    return;
}

void sendLCD(char dato, char rs)
{
    LCDEN = 1;
    LCDPORT = dato;
    LCDRS = rs;
    __delay_ms(3);  //serve per sabilizzare le tensioni, quindi ad evitari eventuali errori
    LCDEN = 0;
    __delay_ms(3);
    LCDEN = 1;
}

void initLCD()
{
    D = 0x00;
    E &= ~0x06;  
    LCDRS = 0;
    LCDEN = 0;
    __delay_ms(20);
    LCDEN = 1;
    sendLCD(L_CFG, COMMAND);    //mando ox38 e dico che è un comando.
    __delay_ms(5);
    sendLCD(L_CFG, COMMAND);
    __delay_ms(1);
    sendLCD(L_OFF, COMMAND);
    sendLCD(L_ON, COMMAND);
    sendLCD(L_CLR, COMMAND);
    sendLCD(L_CUR, COMMAND);    //0x0E Cursore ON, No blink
    sendLCD(L_L1, COMMAND);     //posizionati nella prima casella in alto nel display
}

/*
 void printLCD(char s[])
{
    int i = 0;
    for(i = 0; i<16; i++)
    {
        sendLCD(s[i], DATA);
    }
}
 */

 void printLCDMod(char s[])
{
    int i = 15;
    for(i = 15; i>=0; i--)
    {
        if(s[i] != (char) 0)
        {
           sendLCD(s[i], DATA);

        }
    }
}
 
 void printLCD(char s[])
{
    int i = 0;
    while(s[i] != '\0')
    {
        sendLCD(s[i++], DATA);
    }
}
 
 /*
  * int pot(int numero, int lunghezza)
 {
     int i = 0;
     for(i = 0; i < lunghezza; i++)
     {
         numero *= numero;
     }
     
     return numero;
 }
  */

 
 void convert(int num, char* array)
 {
     short neg = 0;
     int i = 0;
     
     if(num < 0)
     {
         num *= -1;
         neg = 1;
     }
     
     do {
        array[i++] = num % 10 + '0'; //aggiunge 0 al carattere ASCII
        num /= 10;
    } while (num != 0);
    
    array[i++] = '-';
 }

 



    