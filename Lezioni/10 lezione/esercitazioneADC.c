// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define L_ON	0x0F
#define L_OFF	0x08
#define L_CLR	0x01
#define L_L1	0x80
#define L_L2	0xC0
#define L_CR	0x0F		
#define L_NCR	0x0C	
#define L_CFG   0x38
#define L_CUR   0x0E

#define COMMAND 0
#define DATA 1
#define LCDPORT PORTD
#define LCDEN PORTEbits.RE1
#define LCDRS PORTEbits.RE2

#define BUTTONUP PORTBbits.RB1
#define BUTTONDOWN PORTBbits.RB0

#define CLEAR_DISPLAY sendLCD(L_CLR, COMMAND)

#define HEATER PORTCbits.RC5
#define COOLER PORTCbits.RC2

#define _XTAL_FREQ 8000000
#include <xc.h>

void initPicConfig(void);
int readADC(void);
void adcInit(void);
char buttonMatrix(void);
void updateSP(int);
void initTimerInterrupt(void);
void initLCD(void);
void sendLCD(char, char);
void printStringLCD(char *);
void initUart(long int);
void sendUARTString(char *);
char convert(char);//convert set point into it's digital representation
void intToStr(int, char*);
char strToInt(char*);
char NumberLenght(long);
void checkSP(int, char);
void updateTempLCD(int);
void updateSpLCD(char);


int count, read;

char sp = 25; //gradi contigrsdi
char spDigital;
int oldTempDigital, tempDigital;

char dcHeater = 50;
char dcCooler = 50;

char buttonPressed;

char stringTemp[16];
char index, endSend;
char stringSerial[5];

void main(void) {
    TRISB = 0x00;
    TRISD = 0xFF;

    //init all the necessaries methods
    initPicConfig();
    initTimerInterrupt();
    initLCD();
    initUart(9600);
    adcInit();
    
    //Print on LCD initial configuration of data to be displayed
    printStringLCD("Temp: ");
    sendLCD(0xC0, COMMAND);
    printStringLCD("SP: ");
    updateSpLCD(sp);

    while (1) {
        if (endSend) {
            endSend = 0;
            index = 0;
            sp = strToInt(stringSerial) / 2;//output number is too big, so i divide it by 2

            updateSpLCD(sp);

            checkSP(tempDigital, spDigital);
        }

        read = buttonMatrix();

        if (read & 0x08) {
            updateSP(1);
        } else if (read & 0x10) {
            updateSP(-1);
        } else {
            __delay_ms(2);
            buttonPressed = 0;
        }

        int tempDigital = readADC();

        if (oldTempDigital != tempDigital) {
            spDigital = convert(sp);

            updateTempLCD(tempDigital);

            //add inside function
            checkSP(tempDigital, spDigital);
        }

        oldTempDigital = tempDigital;
        //FONDAMENTALE!!!
        __delay_ms(5);
    }
}

void initPicConfig() {
    TRISA = 0xFF; // imposto tutto input
    TRISB = 0x03; // RB0 e RB1 come input, il resto output
    TRISC = 0x00; // tutti output
    TRISD = 0x00; // pilotaggio LCD Output
    TRISE = 0x00; // pilotaggio LCD output
}

void updateTempLCD(int tempDigital) {
    intToStr(tempDigital, stringTemp);
    sendLCD(0x86, COMMAND);
    printStringLCD(stringTemp);
    sendUARTString(" ");
    sendUARTString(stringTemp);
}

void updateSpLCD(char sp) {
    spDigital = convert(sp);
    intToStr(spDigital, stringTemp);
    sendLCD(0xC4, COMMAND);
    printStringLCD(stringTemp);
}

void checkSP(int tempDigital, char spDigital) {
    if (tempDigital > spDigital) {
        //printStringLCD("h");
        //Accendi Cooler (Ventola)
        //PORTC |= 0x04;
        COOLER = 1;
        HEATER = 0;
    } else if (tempDigital < spDigital) {
        //printStringLCD("l");
        //Spegni Cooler (Ventola)
        //PORTC &= 0x04;
        COOLER = 0;
        HEATER = 1;
    } else {
        COOLER = 0;
        HEATER = 0;
    }

}

int readADC() {
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO != 0);
    return ADRESL + (ADRESH << 8);
}

char convert(char sp) {
    return sp * 255 / 77;
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

    //    TRISA = 0xFF;
    //    ADCON0 = 0x11;
    //    ADCON1 = 0x89;
    //    __delay_us(20);
}

char buttonMatrix() {
    //TRISB buttons in input
    //char trisB_bku = TRISB;
    TRISB = 0x3F;

    //TRISB = trisB_bku;

    return ~PORTB & 0x18;
}

void updateSP(int value) {
    if (buttonPressed)return;
    sp += value;
    updateSpLCD(sp);
    //printStringLCD("send");
    buttonPressed = 1;
}

void initTimerInterrupt() {
    INTCON = 0xA0; // abilito GIE e TOIE
    OPTION_REG = 0x08;
    TMR0 = 6;
}

void initLCD() {
    TRISD = 0x00;

    TRISE &= 0xF9;
    /*
     TRISEbits.RE1 = 0;
     TRISEbits.RE2 = 0;
     */
    LCDRS = 0;
    LCDEN = 0;
    __delay_ms(20);
    LCDEN = 1;
    sendLCD(L_CFG, COMMAND);
    __delay_ms(5);
    sendLCD(L_CFG, COMMAND);
    __delay_ms(1);
    sendLCD(L_CFG, COMMAND);
    sendLCD(L_OFF, COMMAND);
    sendLCD(L_ON, COMMAND);
    sendLCD(L_CLR, COMMAND);
    sendLCD(L_CUR, COMMAND); // 0x0E cursor on, no blink
    sendLCD(L_L1, COMMAND);
}

void sendLCD(char dato, char rs) {
    LCDEN = 1;
    LCDPORT = dato;
    LCDRS = rs;
    __delay_ms(3);
    LCDEN = 0;
    __delay_ms(3);
    LCDEN = 1;
}

void printStringLCD(char *s) {
    char i = 0;
    while (s[i] != '\0') {
        sendLCD(s[i++], DATA);
    }
}

void initUart(long int baudRate) {
    TRISC |= 0x80;
    TRISC &= 0x40;

    TXSTA |= 0x20;

    RCSTA |= 0x90;

    INTCON |= 0xC0;

    PIE1 |= 0x20;
    PIE1bits.RCIE = 1;

    SPBRG = (_XTAL_FREQ / (long) (64UL * baudRate)) - 1;
}

void sendUARTString(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        while (!PIR1bits.TXIF);
        TXREG = str[i++];
    }
}

char strToInt(char* s) {
    char num = 0;
    //char length = strLength(s);
    char i = 0;
    char power = 10; //ipotizziamo numeri da soli due cifre

    while (s[i] != '\0') {
        for (char j = 0; j < 10; j++) {
            if (s[i] == (j + 48)) {
                num += j * power;
                break;
            }
        }
        i++;
        power /= 10;
    }

    return num;
}

void intToStr(int num, char* s) {
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
        s[i + 1] = '\0';
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

void __interrupt() ISR() {
    if (T0IF) {
        T0IF = 0;
        TMR0 = 6;
        count++;
        if (count > 100) count = 0;

        //        if (spDigital > tempDigital) {
        //            HEATER = 1;
        //            dcHeater = 100 * (spDigital - tempDigital) / spDigital;
        //        } else {
        //            COOLER = 1;
        //            dcHeater = 100 * (spDigital - tempDigital) / spDigital;
        //        }

        //        if (dcHeater > count) HEATER = 1;
        //        else HEATER = 0;
        //
        //        if (dcCooler > count) COOLER = 1;
        //        else COOLER = 0;
    }

    if (RCIF) {
        RCIF = 0;
        stringSerial[index] = RCREG;
        if (stringSerial[index] == 13) {
            endSend = 1;
            stringSerial[index] = '\0';
        }
        index++;
    }
}