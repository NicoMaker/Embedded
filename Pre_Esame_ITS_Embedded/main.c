#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = ON
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

// Tempo di klock del pic
#define _XTAL_FREQ 8000000
#include <xc.h>
#include <string.h>
#include <stdio.h>


// Configurazione LCD
#define L_ON  0x0F
#define L_OFF 0x08
#define L_CLR 0x01 // CLEAR
#define L_L1  0x80 // LINEA 1
#define L_L2  0xC0 // LINEA 2
#define L_CR  0x0F // Cursore on e lampeggia
#define L_NCR 0x0C //Cursore OFF forse
#define L_CFG 0x38
#define L_CUR 0x0E // CURSOR on NON lampeggia

// Se mando un comando o se mando un dato
#define COMMAND 0
#define DATA 1

// Comando IN OUT all'LCD
#define LCD_PORT PORTD
#define LCD_TRIS TRISD

#define LCD_EN PORTEbits.RE1 //enable clock 
#define LCD_RS PORTEbits.RE2 //rs o zero o 1 (comando/dato)
#define LCD_EN_DIR TRISEbits.TRISE1 //bit configurazione dell'enable del display ( va configurato in output quindi lo imposteremo a 0)
#define LCD_RS_DIR TRISEbits.TRISE2 //bit configurazione del filo RS (Register Select) Verrà popolato da COMMAND o DATA 
#define SETP0 0x01 //imposta ingresso sul primo potenziometro 
#define SETP1 0x09 //imposta ingresso sul secondo potenziometro 
#define SETLM35 0X11 //imposta ingresso sul sensore di temperatura
#define HEATER PORTCbits.RC5
#define COOLER PORTCbits.RC2

void initializationLCD(void); //Inizializzazione LCD
void sendLCD(char, char); //Manda i dati dentro LCD
void printStringLCD(char *); //Printa i dati dentro LCD usando sendLCd
void InitPic(void); //inizializza funzione base del pic
unsigned int readADC(void);
void initADC(void);
int counter = 0;
int AnalogRead = 0;
unsigned int Temperatura;
unsigned int SP = 27;

void main(void) {
    initializationLCD();
    InitPic();
    initADC();
    printStringLCD("Benvenuto");
    __delay_ms(100);
    sendLCD(L_CLR, COMMAND);
    sendLCD(L_L1, COMMAND);
    printStringLCD("SetPoint:   'C");
    sendLCD(L_L2, COMMAND);
    printStringLCD("Temp Ril:   'C");
    TRISC = 0x00;
    HEATER = 1;
    //printStringLCD()
    TRISB = 0X00;
    char x = 0;
    while (1) {

        if (AnalogRead == 1) {
            AnalogRead = 0;
            //Lanciamo funzione che legge il convertitore analogico-digitale ( sensore)
            x++;
            PORTB = x;
            Temperatura = readADC() >> 1; //shifta il valore letto di uno a destra. così leviamo il mezzogrado e ha solo numeri interi 
            char strTr[3]; //dichiaro array di 3 caratteri //stringa temperatura rilevata 
            char strSp[3];
            sprintf(strTr, "%d", Temperatura);
            sprintf(strSp, "%d", SP);
            sendLCD(L_L2 + 10, COMMAND);
            printStringLCD(strTr);
            sendLCD(L_L1 + 10, COMMAND);
            printStringLCD(strSp);
        }
    }
    return;
}

//<editor-fold defaultstate="collapsed" desc="FUNZIONI PER IL DISPLAY LCD">
//SCRITTURA FUNZIONI PER QUANTO RIGURDA IL DISPLAY LCD-----------------------------------------------------------------------------------------------------

//TERZA FUNZIONE, UTILE PER MANDARE I DATI DALLA SERIALE AL PIC E LEGGERLI DENTRO IL DISPLAY LCD

void initializationLCD() //aggiunto dopo per provare ad aggiungere un dato dalla seriale al microcontrollore (RICEZIONE)
{//TUTTE QUESTO CHE HO MESSO HO SEGUITO LA TABELLA SUL FILE WORD
    TRISD = 0x00;
    TRISE &= ~0x06; //PRIMA ABBIAMO FATTO #define LCDEN PORTEbits.RE1(RE1 = EN)  #define LCDRS PORTEbits.RE2 (RE2 = RS) 
    //sarebbe TRISEbits.RE1 = 0; (RE1 = EN)     //sarebbe TRISEbits.RE2 = 0; (RE2 = RS)       6->0110 CORRISPONDE A METTERE A 0 1001 CHE SAREBBERO RE1/RE2 = 0 CIOE' RS ED EN !! IL FATTO DI METTERE RE1/RE2 A ZERO CE LO IMPONE LA TABELLA DI CONFIGURAZIONE

    //ORA SEGUIAMO PARI PARI LA TABELLA DI INIZIALIZZAZIONE USANDO APPUUNTO TUTTE LE VARIABILE DEFINE CHE ABBIAMO INIZIALIZZATO SOPRA CON TUTTI I VALORI PRESI DAL FILE WORD
    LCD_RS = 0;
    LCD_EN = 0;
    __delay_ms(20);
    LCD_EN = 1;
    sendLCD(L_CFG, COMMAND); //sono i COMANDI da inviare dentro la funzione sendLCD COMMAND ? PARI A 0; L_CGF ? il comando che mandiamo appunto perch? RS = 0
    __delay_ms(5);
    sendLCD(L_CFG, COMMAND);
    __delay_ms(1);
    sendLCD(L_CFG, COMMAND);
    sendLCD(L_OFF, COMMAND);
    sendLCD(L_ON, COMMAND);
    sendLCD(L_CLR, COMMAND);
    sendLCD(L_NCR, COMMAND); // prima era scritto L_CUR e abilitava il cursore, ora lo disabilita.
    sendLCD(L_L1, COMMAND); //L1 POSIZIONE DELLA PRIMA CASELLA

}

//QUARTA FUNZIONE

void sendLCD(char dato, char rs) //aggiunto dopo per provare ad aggiungere un dato dalla seriale al microcontrollore (RICEZIONE)
{
    LCD_EN = 1;
    LCD_PORT = dato;
    LCD_RS = rs;
    __delay_ms(3);

    LCD_EN = 0;
    __delay_ms(3);

    LCD_EN = 1;
}


//QUINTA FUNZIONE, i dati che mi arrivano li devo inviare dentro il diplay lcd

void printStringLCD(char *s) { //aggiunto dopo per provare ad aggiungere un dato dalla seriale al microcontrollore (RICEZIONE)
    char i = 0;
    while (s[i] != '\0') { //STAMPA FINO A QUANDO NON BECCA \0
        sendLCD(s[i++], DATA); //mandiamo un dato dentro la memoria del display LCD
    }
}
//FINE SCRITTURA FUNZIONI PER QUANTO RIGURDA IL DISPLAY LCD-----------------------------------------------------------------------------------------------------
//</editor-fold>

void InitPic() {
    INTCON = 0XE0; //serve per accendere il sistema INTCON //abilita GIE (int gen edificio), PEIE, T0IE(interrutt stanza)  (ppt 2 Slide 10) (T0IE abilita il singolo timer)  (PEIE servirà per la seriale)
    OPTION_REG = 0X03;
    TMR0 = 131;
    TRISA = 0xFF;

}

unsigned int readADC() {
    ADCON0bits.GO = 1; //ADCON0 |= 0x04; // 
    while (ADCON0bits.GO != 0); // Attendi fine conversione
    unsigned int temp = ((ADRESH << 8) + ADRESL); // Calcola valore temperatura
    return temp;
}

void initADC() {
    ADCON1 = 0x80; // Configura AN2 come analogico e VDD come Vref
    ADCON0 = SETLM35; // Abilita l'ADC e seleziona AN2 //LEGGI SENSORE DI TEMPERATURA 

}

void initUART(unsigned long baudRate) {
    TRISCbits.TRISC7 = 1;
    TRISCbits.TRISC6 = 0;

    TXSTAbits.TXEN = 1;
    RCSTAbits.SPEN = 1;
    RCSTAbits.CREN = 1;
    SPBRG = (char) (_XTAL_FREQ / (unsigned long) (64UL + baudRate)) - 1;
}

void __interrupt() ISR() {
    if (T0IF) { //if (T0IF ==1)
        TMR0 = 131;
        counter++;
        if (counter > 100) {
            counter = 0;
            AnalogRead = 1;
        }
        T0IF = 0;

    }
}