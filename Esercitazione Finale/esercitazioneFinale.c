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

#define HEATER PORTCbits.RC5 //HEATER
#define COOLER PORTCbits.RC2 //COOLER

#define MSG_TEMP 0x01
#define MSG_TH 0x02
#define MSG_TL 0x03
#define MSG_End 0x00

#define _XTAL_FREQ 8000000
#include <xc.h>

void initPicConfig(void);
double readADC(void);
void checkSP(double, char);
void initADC(void);
void configLCD(void);
void printParamsLCD(void);
char buttonMatrix(void);
void identifyButton(char);
void initTimerInterrupt(void);
void initLCD(void);
void sendLCD(char, char);
void printStringLCD(char *);
void printStringLCDOffset(char *, char, char); //char[], start, stop
void initUart(long int);
void sendUARTString(char *);
char convert(char); //convert set point into it's digital representation
void intToStr(int, char*);
char strToInt(char*);
char NumberLenght(long);
void unpackMessage(char *);
void printTemperatureLCD(int, int);


//timer interrupt
int count;
char readTempFlag;

char read, oldRead;

char buttonPressed;

char stringTemp[16];
char stringTempo[5];
char indexSerial, endSend;
char msgSerial[16];
char receiveSerial[16];

double oldTemperature, temperature;
char temperatureSP;
int tempo;
char canDecrement;
char endCook;
int countRestart;

char machineStatus;

//function array
void(*funcArray[4])();

void func0() {
    sendLCD(0x01, COMMAND);
    //print waiting params chars
    configLCD();
}

void func1() {
    printStringLCD("func2");
}

void func2() {

}

void func3() {

}

void main(void) {
    TRISB = 0x00;
    TRISD = 0xFF;

    funcArray[0] = func0;
    funcArray[1] = func1;
    funcArray[2] = func2;
    funcArray[3] = func3;


    //init all the necessaries methods
    initPicConfig();
    initTimerInterrupt();
    initLCD();
    initUart(9600);
    initADC();

    //    //config LCD
    //configLCD();

    //close heater
    HEATER = 0;
    while (1) {
        switch (machineStatus) {
            case 0:

                funcArray[machineStatus]();

                machineStatus = 99;
                break;
            case 1:
                //print params visualization
                unpackMessage(receiveSerial);
                printParamsLCD();
                machineStatus = 99;
                break;
            case 2:
                if (!tempo) machineStatus = 3;

                temperature = readADC() * 0.488;
                int pI = (int) temperature;
                int pD = (temperature - pI) * 100;

                if (oldTemperature != temperature) {
                    printTemperatureLCD(pI, pD);
                    checkSP(temperature, temperatureSP);
                }

                oldTemperature = temperature;

                //print time decrementation
                intToStr(tempo, stringTempo);
                sendLCD(0xC7, COMMAND);
                printStringLCD(stringTempo);
                printStringLCD(" ");
                break;
            case 3:
                sendLCD(0x01, COMMAND);
                printStringLCD("Cooked!");
                HEATER = 0;
                endCook = 1;
                canDecrement = 0;
                machineStatus = 99;
                break;
            default:
                break;
        }

        if (endSend) {
            endSend = 0;
            indexSerial = 0;
            machineStatus = 1;
        }

        read = buttonMatrix();

        if (oldRead != read) {//if theese valuea are dfferent, the curent button pressed is not pressed anymore
            identifyButton(read);
        }

        oldRead = read;
    }
}

void unpackMessage(char * msg) {
    temperatureSP = ((msg[2] - '0') * 10) + (msg[3] - '0');
    //temperatureSP << 1;
    //if ((msg[5] - '0') == 5)temperatureSP++;

    char index;
    while (msg[index++] != '\0');

    index -= 2;
    //char index = sizeof(msg) - 2;
    char iteration = 1;
    while (msg[index] != ':') {
        tempo += (msg[index--] - '0') * iteration;
        iteration *= 10;
        //index--;
    }
    //tempo = (msg[8] - '0') * 1000 + (msg[9] - '0') * 100 + (msg[10] - '0') * 10 + (msg[11] - '0');
}

void printStringLCDOffset(char *s, char start, char stop) {
    if (start > stop) return;

    while (s[start] != '\0' && start <= stop) {
        sendLCD(s[start++], DATA);
    }
}

void checkSP(double temp, char sp) {
    if (temp >= sp + 1) {
        COOLER = 1;
        HEATER = 0;
    } else if (temp < sp - 1) {
        COOLER = 0;
        HEATER = 1;
    } else {
        canDecrement = 1;
        COOLER = 0;
        HEATER = 0;
    }
}

void configLCD() {
    printStringLCD("In attesa");
    sendLCD(0xC0, COMMAND);
    printStringLCD("di parametri");
    sendLCD(0x80, COMMAND);
    sendLCD(0x0C, COMMAND); // disable cursor
}

void printTemperatureLCD(int pI, int pD) {
    intToStr(pI, stringTemp);
    sendLCD(0x86, COMMAND);
    printStringLCD(stringTemp);

    printStringLCD(".");

    intToStr(pD, stringTemp);
    sendLCD(0x89, COMMAND);
    printStringLCD(stringTemp);
    sendLCD(0xDF, DATA);
    printStringLCD("C  ");
}

void printParamsLCD() {
    //first line
    sendLCD(0x80, COMMAND);
    printStringLCD("Temp:      ");
    sendLCD(0xDF, DATA);//°
    printStringLCD("C");
    sendLCD(0x86, COMMAND);
    printStringLCDOffset(receiveSerial, 2, 5);

    //second line
    sendLCD(0xC0, COMMAND);
    printStringLCD("Tempo:     s");
    sendLCD(0xC7, COMMAND);
    printStringLCDOffset(receiveSerial, 8, 11);
}

void initPicConfig() {
    TRISA = 0xFF; // imposto tutto input
    TRISB = 0x03; // RB0 e RB1 come input, il resto output
    TRISC = 0x00; // tutti output
    TRISD = 0x00; // pilotaggio LCD Output
    TRISE = 0x00; // pilotaggio LCD output
}

double readADC() {
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO != 0);
    return ADRESL + (ADRESH << 8);
}

void initADC() {
    //Almeno un ingresso in input
    TRISA = 0xFF; //check later for interrupt issue...

    //Imposto RC5 come output per mettere in output l'heater
    TRISC &= ~0x20;

    //CH0, CH1, CH2 impostati a zero per selezionare il primo canale
    ADCON0 |= 0x11;
    ADCON1 |= 0x80;

    //Per vedere uqali bit sono associati a quali componenti della macchina, guarda i pulsantini bianchi sul simulatore (non serve vedere schema elettrico)
    //Accendi heater
    PORTC |= 0x20;

    __delay_us(20);
}

char buttonMatrix() {
    //TRISB buttons in input
    //char trisB_bku = TRISB;
    //char trisB_bku = TRISB;
    TRISB = 0x3F;
    char read = ~PORTB & TRISB;
    if (read) {
        __delay_us(50);
        read = ~PORTB & TRISB;
        if (read) {
            buttonPressed = 1;
            return read;
        }
    }

    buttonPressed = 0;
    return 0;
}

void identifyButton(char read) {
    if (read & 0x08) {
        machineStatus = 2;
    } else {
        buttonPressed = 0;
    }
}

void initTimerInterrupt() {
    INTCON = 0xA0; // abilito GIE e TOIE
    OPTION_REG = 0x02;
    TMR0 = 6;
}

void initLCD() {
    TRISD = 0x00;

    TRISE &= 0xF9;

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
        //Sottraggo al numero un valore sempre più grande per calcolare la lunghezza del numero di input.
        baseTester = baseTester * 10 + 9;
        numLength++;
    }
    return numLength;
}

void __interrupt() ISR() {
    if (T0IF) {
        TMR0 = 6;

        if (++count >= 1000) {
            count = 0;
            if (canDecrement) tempo--;
        }

        if (endCook) {
            if (countRestart++ > 5000) {
                machineStatus = 0;
                endCook = 0;
                countRestart = 0;
            }
        }

        T0IF = 0;
    }

    if (RCIF) {
        receiveSerial[indexSerial] = RCREG;
        if (receiveSerial[indexSerial] == 13) {
            endSend = 1;
            receiveSerial[indexSerial] = '\0';
        }
        indexSerial++;
        RCIF = 0; //setta a 0 per evitare che il tempo di esecuzione del codice nell'interrupt porti all'avvenire di un altro interrupt
    }
}
