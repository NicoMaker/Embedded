#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
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

#define COMMAND 0
#define DATA 1

#define LCD_PORT PORTD
#define LCD_TRIS TRISD

#define LCD_EN PORTEbits.RE1
#define LCD_RS PORTEbits.RE2

#define _XTAL_FREQ 8000000
#include <xc.h>

void INITUART(int);
void initLCD(void);
void SENDUARTSTRING(char *);
void sendLCD(char,char);

char receivedString[10], indexString, endReceive;

void main(void) {

    INITUART(9600);

    while (1) {
        
        if(endReceive){
            SENDLCDSTRING(receivedString);
            endReceive = 0;
            indexString = 0;
        }

        SENDUARTSTRING("ciao");
        __delay_ms(1000);
    }
}

void INITUART(int baudRate) // trasmisione e ricezione seriale
{
    TRISCbits.TRISC7 = 1;
    TRISCbits.TRISC6 = 0;

    //    TRISC |= 0x80;
    //    TRISC = 0x00 | 0x80;
    //    TRISC &= ~0x40;

    TXSTAbits.TXEN = 1;
    // TXSTA |= 0x20; trasmetto trasmettittore 

    // RCSTAbits.CREN = 1;
    // RCSTAbits.SPEN = 1;

    RCSTA |= 0x90; // trasmetto ricevitore

    INTCON |= 0xC0;
    PIE1 |= 0x20;

    SPBRG = (_XTAL_FREQ / (long) (64UL * baudRate)) - 1; // uso variabile del calcolo senza segno UL
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

void SENDUARTSTRING(char * string) { // inviare i dati della stringa alla seriale 

    char i = 0;

    while (string[i] != '\0') {
        
        while (!PIR1bits.TXIF);
        TXREG = string[i++];
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

void __interrupt() ISR(){

    if(RCIF){ //se sta ricevendo
        RCIF = 0;
        
        receivedString[indexString] = RCREG;

        if(receivedString[indexString] == 13) {// 13 -> valore che corrisponde a \0 e CR
            receivedString[indexString] = '\0';
            endReceive = 1;
        }    
        
        indexString++;
    }
}