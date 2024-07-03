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
#define PICID 13

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
void initUART(unsigned long);
void sendUARTString(char *);
void SendnumerUart(unsigned char);
void SendValues(void);
int toint(char *);
int counter = 0;
int AnalogRead = 0;
unsigned char Temperatura;
unsigned char SP = 27;
unsigned char SerialIn[20];
unsigned int SerialIndex = 0;
unsigned int DataReceived = 0;
unsigned char Heater_status;
unsigned char Cooler_status;
char oldtemp;

void main(void) {
    initializationLCD();
    InitPic();
    initADC();
    TRISC = 0x00;
    initUART(115200);
    printStringLCD("Ciao");
    __delay_ms(100);
    sendLCD(L_CLR, COMMAND);
    sendLCD(L_L1, COMMAND);
    printStringLCD("SetPoint:  C");
    sendLCD(L_L2, COMMAND);
    printStringLCD("Temp Ril:  C");

    //printStringLCD()
    TRISB = 0X00;
    char x = 0;

    while (1) {

        if (AnalogRead == 1) {
            AnalogRead = 0;
            //Lanciamo funzione che legge il convertitore analogico-digitale ( sensore)
            //x++;
            //PORTB=x;
            Temperatura = readADC() >> 1; //shifta il valore letto di uno a destra. così leviamo il mezzogrado e ha solo numeri interi 
            char strTr[3]; //dichiaro array di 3 caratteri //stringa temperatura rilevata 
            char strSp[3];
            sprintf(strTr, "%d", Temperatura);
            sprintf(strSp, "%d", SP);
            if (Temperatura != oldtemp) {
                SendValues();
                oldtemp = Temperatura;
            }
            sendLCD(L_L2 + 9, COMMAND);
            printStringLCD(strTr);
            sendLCD(L_L1 + 9, COMMAND);
            printStringLCD(strSp);

            sprintf(strSp, "H:%d", Heater_status);
            sendLCD(L_L2 + 13, COMMAND);
            printStringLCD(strSp);

            sprintf(strSp, "C:%d", Cooler_status);
            sendLCD(L_L1 + 13, COMMAND);
            printStringLCD(strSp);
        }
        if (DataReceived == 1) {
            DataReceived = 0;
            //sendLCD(L_CLR, COMMAND); //STAMPANO QUELLO CHE ARRIVA SUL DISPLAY
            //sendLCD(L_L1, COMMAND);
            //printStringLCD(SerialIn);
            SP = (SerialIn[0] - '0') * 10 + (SerialIn[1] - '0');


        }

        if (Temperatura < SP) {
            HEATER = 1;
            COOLER = 0;
            Heater_status = 1;
            Cooler_status = 0;
        } else if (Temperatura > SP) {
            HEATER = 0;
            COOLER = 1;
            Heater_status = 0;
            Cooler_status = 1;
        } else {
            HEATER = 0;
            COOLER = 0;
            Heater_status = 0;
            Cooler_status = 0;
        }
        if (DataReceived){
            char text = toint(SerialIn);
            SP = text;
            DataReceived = 0;
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
    PIE1bits.RCIE = 1;
}

void sendUARTString(char * s) //il puntatore va a puntare una zona di memoria dove è presente un array di char
{
    char i = 0;
    while (s[i] != '\0') //fintanto che il char che stai puntando è diverso da \0 (ovvero la fine dell'array) fai:
    {
        while (!PIR1bits.TXIF); //fintanto che sta trasmettendo il dato resterà su questa istruzione
        TXREG = s[i++]; //esce dal ciclo, assegna, incrementa, ricomincia
    }
}

void SendnumerUart(unsigned char temp) {
    if (temp < 10) {
        TXREG = '0' + temp % 10;
    } else {
        TXREG = '0' + (temp / 10) % 10;
        __delay_ms(5);
        TXREG = '0' + temp % 10;
    }
}

void SendValues() {

    SendnumerUart(PICID);
    __delay_ms(5);
    sendUARTString(",");
    __delay_ms(5);
    SendnumerUart(Temperatura);
    __delay_ms(5);
    sendUARTString(",");
    __delay_ms(5);
    SendnumerUart(SP);
    __delay_ms(5);
    sendUARTString(",");
    SendnumerUart(Heater_status);
    __delay_ms(5);
    sendUARTString(",");
    SendnumerUart(Cooler_status);
    __delay_ms(5);

    sendUARTString("\r");
}

int toint(char * s){
    int num = (s[0] - '0') * 10 + (s[1] - '0');
    return num;
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

    if (RCIF) {
        RCIF = 0;
        PORTBbits.RB0 = 1;
        SerialIn[SerialIndex] = RCREG;
        if (SerialIn[SerialIndex] == 13) {
            DataReceived = 1;
            SerialIn[SerialIndex] = '\0';
            SerialIndex = 0;

        } else SerialIndex++;

    }
}