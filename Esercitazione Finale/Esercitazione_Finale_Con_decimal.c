#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF  

#define _XTAL_FREQ 8000000

#include <xc.h>

#define L_ON  0x0F
#define L_OFF 0x08
#define L_CLR 0x01 // CLEAR
#define L_L1  0x80 // LINEA 1
#define L_L2  0xC0 // LINEA 2
#define L_CR  0x0F // Cursore on e lampeggia
#define L_NCR 0x0C
#define L_CFG 0x38
#define L_CUR 0x0E // CURSOR on NON lampeggia

#define COMMAND 0
#define DATA 1

#define LCD_EN PORTEbits.RE1 //enable clock 
#define LCD_RS PORTEbits.RE2 //rs o zero o 1 (comando/dato)

#define LCD_PORT PORTD
#define LCD_TRIS TRISD

#define SETLM35 0X11 //imposta ingresso sul sensore di temperatura

#define HEATER PORTCbits.RC5 //HEATER

void initPic(void);

void initADC(void);
unsigned int readADC(void);

void initUART(long int); //Inizializzazione porta Seriale
void receiveUARTstring(void);

void extractTimeFromUART(char*, char*);
int extractLengthFromTimeString(char*);
int extractPower(char);
int timeStringToTimeInt(char*);

void inizializationLCD(void); //Inizializzazione LCD
void sendLCD(char, char); //Manda i dati dentro LCD
void printStringLCD(char *); //Printa i dati dentro LCD usando sendLCd

void ConvertIntToCharForLCD(char *, int);
void printLCDParams(char *, char *, char*);


unsigned int i = 0;
unsigned int interruptCounter;
unsigned int index;
unsigned int endSend;
double setPoint = 0;
double temperature;
unsigned int temperatureDecimal;
unsigned int oldX;
unsigned int oldY;
unsigned int time = 0;
unsigned int oldT;
unsigned int analogRead = 0;
unsigned int tControl = 0;
unsigned int firstPrint = 1;

unsigned int x;
unsigned int y;
unsigned int t;

char serialInString[20];
char strTemperatureValue[5];
char strTemperatureDecimalValue[5];
char strTimeValue[6];
char strSetPoint[5];

void main(void) {

    initPic();
    initADC();
    inizializationLCD();
    initUART(9600);

    __delay_us(20);

    while (1) {

        receiveUARTstring();
        
        if (analogRead == 1) {

            analogRead = 0;

            //receiveUARTstring();

            temperature = readADC() * 0.488;
        }
        if (i == 1) {
            if (~PORTB & 0x01) {

                HEATER = 1;
                i = 2;

            }
        }

        switch (i) {

            case 0:

                if (firstPrint == 1)
                {
                //sendLCD(L_CLR, COMMAND);
                sendLCD(L_L1, COMMAND);
                printStringLCD("In attesa  ");
                sendLCD(L_L2, COMMAND);
                printStringLCD("di parametri   ");
                firstPrint = 0;
                }
                break;

            case 1:

                x = (int)setPoint;
                y = (setPoint - x) * 100;
                t = time;

                break;

            case 2:

                x = (int)temperature;
                y = (temperature - x) * 100;
                t = time;
                if (temperature >= setPoint) {
                    i = 3;
                }

                break;

            case 3:

                tControl = 1;
                x = (int)temperature;
                y = (temperature - x) * 100;
                t = time;
                if (temperature >= setPoint) {
                    HEATER = 0;
                } else {
                    HEATER = 1;
                }
                
                if (time == 0){
                    i = 0;
                    HEATER = 0;
                    tControl = 0;
                    firstPrint = 1;
                    time = 0;
                    setPoint = 0;
                    index = 0;
                }
                break;
        }

        if ((oldX != x) || (oldT != t) || (oldY != y)) {
            ConvertIntToCharForLCD(strTemperatureValue, x);
            ConvertIntToCharForLCD(strTimeValue, t);
            ConvertIntToCharForLCD(strTemperatureDecimalValue, y);
            printLCDParams(strTemperatureValue, strTemperatureDecimalValue, strTimeValue);
        }
        oldX = x;
        oldY = y;
        oldT = t;
        

    }

    return;
}

void initPic(void) {

    INTCON = 0XE0; //serve per accendere il sistema INTCON //abilita GIE (int gen edificio), PEIE, T0IE(interrutt stanza)  (ppt 2 Slide 10) (T0IE abilita il singolo timer)  (PEIE servir? per la seriale)
    OPTION_REG = 0X03;
    TMR0 = 131;

    TRISC = 0x00;

    TRISB = 0x0F;
    PORTB = 0x0F;

    TRISD = 0x00;
    TRISA = 0xFF;
}


//<editor-fold defaultstate="collapsed" desc="FUNZIONI PER IL DISPLAY LCD">
//SCRITTURA FUNZIONI PER QUANTO RIGURDA IL DISPLAY LCD-----------------------------------------------------------------------------------------------------

//TERZA FUNZIONE, UTILE PER MANDARE I DATI DALLA SERIALE AL PIC E LEGGERLI DENTRO IL DISPLAY LCD

void inizializationLCD() //aggiunto dopo per provare ad aggiungere un dato dalla seriale al microcontrollore (RICEZIONE)
{//TUTTE QUESTO CHE HO MESSO HO SEGUITO LA TABELLA SUL FILE WORD
    //TRISD = 0x00;
    TRISE &= ~0x06; //PRIMA ABBIAMO FATTO #define LCDEN PORTEbits.RE1(RE1 = EN)  #define LCDRS PORTEbits.RE2 (RE2 = RS) 
    //sarebbe TRISEbits.RE1 = 0; (RE1 = EN)     //sarebbe TRISEbits.RE2 = 0; (RE2 = RS)       6->0110 CORRISPONDE A METTERE A 0 1001 CHE SAREBBERO RE1/RE2 = 0 CIOE' RS ED EN !! IL FATTO DI METTERE RE1/RE2 A ZERO CE LO IMPONE LA TABELLA DI CONFIGURAZIONE

    //ORA SEGUIAMO PARI PARI LA TABELLA DI INIZIALIZZAZIONE USANDO APPUUNTO TUTTE LE VARIABILE DEFINE CHE ABBIAMO INIZIALIZZATO SOPRA CON TUTTI I VALORI PRESI DAL FILE WORD
    LCD_RS = 0;
    LCD_EN = 0;
    __delay_ms(20);
    LCD_EN = 1;
    sendLCD(L_CFG, COMMAND); //sono i COMANDI da inviare dentro la funzione sendLCD COMMAND � PARI A 0; L_CGF � il comando che mandiamo appunto perch� RS = 0
    __delay_ms(5);
    sendLCD(L_CFG, COMMAND);
    __delay_ms(1);
    sendLCD(L_CFG, COMMAND);
    sendLCD(L_OFF, COMMAND);
    sendLCD(L_ON, COMMAND);
    sendLCD(L_CLR, COMMAND);
    sendLCD(L_NCR, COMMAND);
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

void printLCDParams(char *strTemperatureValue, char *strTemperatureDecimalValue, char *strTimeValue) {
    //sendLCD(L_CLR, COMMAND);
    sendLCD(L_L1, COMMAND);
    printStringLCD("Temp:     ");
    sendLCD(L_L1 + 6, COMMAND);
    printStringLCD(strTemperatureValue);
    sendLCD(L_L1 + 8, COMMAND);
    printStringLCD(".");
    sendLCD(L_L1 + 9, COMMAND);
    printStringLCD(strTemperatureDecimalValue);
    sendLCD(0xdf, DATA);
    //sendLCD(L_L1 + 9, COMMAND);
    printStringLCD("C");
    sendLCD(L_L2, COMMAND);
    printStringLCD("Tempo: ");
    sendLCD(L_L2 + 7, COMMAND);
    printStringLCD(strTimeValue);
    //sendLCD(L_L2 + 12, COMMAND);
    printStringLCD("s    ");
}

void ConvertIntToCharForLCD(char *str, int valueTempOrTime) {
    if (valueTempOrTime / 1000 > 0) {
        str[0] = (valueTempOrTime / 1000) + '0';        // Thousands place
        str[1] = ((valueTempOrTime / 100) % 10) + '0';  // Hundreds place
        str[2] = ((valueTempOrTime / 10) % 10) + '0';   // Tens place
        str[3] = (valueTempOrTime % 10) + '0';          // Units place
        str[4] = '\0';                                  // Null-terminate
    } else if (valueTempOrTime / 100 > 0) {
        str[0] = (valueTempOrTime / 100) + '0';         // Hundreds place
        str[1] = ((valueTempOrTime / 10) % 10) + '0';   // Tens place
        str[2] = (valueTempOrTime % 10) + '0';          // Units place
        str[3] = '\0';                                  // Null-terminate
    } else if (valueTempOrTime / 10 > 0) {
        str[0] = (valueTempOrTime / 10) + '0';          // Tens place
        str[1] = (valueTempOrTime % 10) + '0';          // Units place
        str[2] = '\0';                                  // Null-terminate
    } else {
        str[0] = (valueTempOrTime % 10) + '0';          // Units place
        str[1] = '\0';                                  // Null-terminate
    }
}



//<editor-fold defaultstate="collapsed" desc="FUNZIONI PER LA COMUNICAZIONE SERIALE">
//SCRITTURA FUNZIONI PER QUANTO RIGUARDA LA COMUNICAZIONE SERIALE-------------------------------------------------------------------------------------------------------------

//PRIMA  FUNZIONE---Funzione per inizializzare la seriale UART, E PER MANDARE I DATI DAL PIC ALLA SERIALE

void extractTimeFromUART(char *strIn, char *strTime)
{
    int timeIndex = 8;
    int y = 0;
    while ( strIn[timeIndex+y] != '\0')
    {
        strTime[timeIndex - 8+y] = strIn[timeIndex+y];
        y++;
    }
    strTime[timeIndex - 8 + y] = '\0';
}

int extractLengthFromTimeString(char * strTime)
{
    int lenghtIndex = 0;
    
    while (strTime[lenghtIndex] != '\0')
    {
        lenghtIndex++;
    }
    
    return lenghtIndex;
}

int extractPower(char e)
{
    int r = 1;
    
    for (int i = 0; i < e; i++)
    {
        r = r * 10;
    }
    
    return r;
}

int timeStringToTimeInt(char *baseString)
{   
    int result = 0;
    int stringLength = extractLengthFromTimeString(baseString);
    
    for (int i = 0; i < stringLength; i++)
    {
        result = result + ((baseString[i] - '0') * extractPower(stringLength - 1 - i));
    }
    
    return result;
}



void initUART(long int baudRate) {

    TRISC |= 0x80; // forziamo ad 1 l'ottavo bit che corrisponde sullo schema elettrico ad RC7---->TX

    // |1 0| 0 0 0 0 0 0
    // TRISC = 0x80;

    TRISC &= ~0x40; // pongo a 0 solo il bit sulla posizione 7 e sullo schema elettrico corrisponde a RC6------>RX

    //        //0x40 => 0100 0000
    //        //~0x40 = 0xBF => 1011 1111

    //facendo and con il primo TRISC ed il secondo TRISC =  1000 0000 end con 1011 1111 --> 1000 0000 IN QUESTO ABBIAMO TX POSTO A 1 E RX A ZERO
    //TRISCbits.TRISC7 = 1; //OPPURE AL POSTO DI FARE COME SOPRA USO QUESTO MODO, CHE ? SIMILE A QUELLO CHE ABBIAMO USATO PER IMPOSTARE RS/EN CON IL DISPLAY LCD
    //TRISCbits.TRISC6 = 0;
    TXSTA |= 0x20; //STA e UN REGISTRO, TX PER TRASMISSIONE, abiliti txen
    //TXSTAbits.TXEN = 1; // Abilito il sottomodulo TX, TRASMISSIONE
    //DOBBIAMO IMPOSTARE DUE BIT A 1, E SI FA COSI
    //RCSTA |= 0x90;    // Per abilitare l'utilizzao dell' UART che corrisponde a quello che c'� scritto sotto, 0x09--> 0000 | 1001
    RCSTAbits.SPEN = 1; //SPEN / CREN sono dei bit dentro il registro di RICEZIONE
    RCSTAbits.CREN = 1; // Abilito ricevimento continuo dei dati
    //INTCON |= 0xC0; per l'interrupt 
    //INTCONbits.GIE = 1; // Abilito il bit GIE del registro INTCON
    //INTCONbits.PEIE = 1; // Abilito il bit PEIE del registro INTCON
    //NON USIAMO OPTION REG PERCH? LUI USA IL PRESCALLER PER SETTARE IL TMR0, IN QUESTO CASO NON SERVE PERCHE NON ABBIAMO UN tempO PRESTABILITO; QUANDO E SOLO QUANDO ARRIVANO I DATI SCATTA L'INTERRUPT (senza alcun tempo prestabilito appunto)
    //QUINDI USIAMO SOLO INTCOIN
    //PIE1 |= 0x20;  
    PIE1bits.RCIE = 1; // Abilito interrupt della ricezione seriale
    //CONFIGURIAMO LA FREQUENZA DI INVIO E RICEZIONE DEI DATI
    SPBRG = (_XTAL_FREQ / (long) (64UL * baudRate)) - 1UL; // 64UL => unsigned long a 64 bit  
}

void receiveUARTstring(void) {
    if (endSend) {
        
        setPoint = ((serialInString[2] - '0')* 10 + (serialInString[3] - '0') + (serialInString[5] - '0') * 0.1);
        
        extractTimeFromUART(serialInString, strTimeValue);
        
        time = timeStringToTimeInt(strTimeValue);
        
        
        endSend = 0;
        
        //index = 0;
        i = 1;

    }
}

//FINE SCRITTURA FUNZIONI PER QUANTO RIGUARDA LA COMUNICAZIONE SERIALE-------------------------------------------------------------------------------------
//</editor-fold>

void initADC(void) {
    //Abilito LM35
    ADCON0 = SETLM35;

    //Abilito potenziometro zero
    //ADCON0= SETP0;

    //Abilito potenziometro uno
    //ADCON0 = SETP1;

    ADCON1 = 0x80;
}

unsigned int readADC(void) {
    ADCON0 |= 0x04; //ADCON0bits.GO = 1 SAREBBE IL GO PER LA CONVERSIONE DA ANA A DECI

    while (ADCON0bits.GO != 0);

    unsigned int temp = (ADRESL + (ADRESH << 8)); //UNISCI I DUE BYTE E TROVI IL VALORE DELLA tempERATURA
    // * 0.488

    return temp;
}

void __interrupt() ISR() { //dalla seriale al microcontrollore--RICEZIONE

    //if (TOIF)
    //{TOIF = 0}
    //TOIF ? stato messo a 1 il bit di overflow (255 --> 256) COSI SCATTA L'OVERFLOW, poi si deve mettere a zero perch? non ha lo stop automatico
    if (T0IF) { //if (T0IF ==1)
        TMR0 = 131;
        interruptCounter++;
        if (interruptCounter > 1000) {
            interruptCounter = 0;
            analogRead = 1;

            if (tControl == 1) {
                time--;
            }
        }
        T0IF = 0;

    }


    if (RCIF) //quando IL BIT ? a 1 vuol dire che stiamo ricevendo dei dati, in questo caso dal terminale al microcontrollore
        //NOI USIAMO RCIF IL BIT PER LA RICEZIONE, NON ABBIAMO T0IF    
    {
        serialInString[index] = RCREG; //il registro rcreg contiene il carattere che li arriva, INVECE PER LA TRASMISSIONE USIAMO TXREG
        if (serialInString[index] == 13) { //sostituisce il valore 13 con /0
            serialInString[index] = '\0'; //QUESTA STRINGA CREATA LA INVIEREMO DENTRO IL DISPLAY LCD
            endSend = 1; //modo per capire che siamo arrivati alla fine della ricezione
        }
        index++;
        RCIF = 0; //? lo stesso discorso di T0IF = 0, questo per essere sensibili al prossimo interrupt
    }

}
