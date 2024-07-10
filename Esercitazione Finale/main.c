#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = ON
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 8000000
#include <xc.h>
#include <string.h>
#include <stdio.h>

// Configurazione LCD
#define L_ON  0x0F
#define L_OFF 0x08
#define L_CLR 0x01
#define L_L1  0x80
#define L_L2  0xC0
#define L_CR  0x0F
#define L_NCR 0x0C
#define L_CFG 0x38
#define L_CUR 0x0E
#define PICID 13

#define COMMAND 0
#define DATA 1

#define LCD_PORT PORTD
#define LCD_TRIS TRISD

#define LCD_EN PORTEbits.RE1
#define LCD_RS PORTEbits.RE2
#define LCD_EN_DIR TRISEbits.TRISE1
#define LCD_RS_DIR TRISEbits.TRISE2

#define SETLM35 0X11
#define HEATER PORTCbits.RC5
#define COOLER PORTCbits.RC2

void initializationLCD(void);
void sendLCD(char, char);
void printStringLCD(char *);
void InitPic(void);
unsigned int readADC(void);
void initADC(void);
void initUART(unsigned long);
void sendUARTString(char *);
void SendnumerUart(unsigned char);
void SendValues(void);
int toint(char *);
void startCooking(void);

unsigned char Temperatura;
unsigned char SP = 27;
unsigned int TimeH = 0;
unsigned int TimeL = 0;
unsigned char SerialIn[20];
unsigned int SerialIndex = 0;
unsigned int DataReceived = 0;
unsigned char Heater_status;
unsigned char Cooler_status;
char oldtemp;
unsigned long CookingTime;
unsigned char CookingInProgress = 0;
unsigned char TargetReached = 0;
unsigned char AnalogRead = 0;
unsigned int counter = 0;
unsigned char strSp;

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

    while (1) {
        if (AnalogRead == 1) {
            AnalogRead = 0;
            Temperatura = readADC() >> 1;
            char strTr[4]; // Incrementato a 4 per gestire numeri a tre cifre e il terminatore nullo
            char strSp[4]; // Incrementato a 4 per gestire numeri a tre cifre e il terminatore nullo
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

            if (CookingInProgress) {
                sendLCD(L_L2 + 13, COMMAND);
                sprintf(strSp, "T:%lu", CookingTime);
                printStringLCD(strSp);
            } else {
                sprintf(strSp, "H:%d", Heater_status);
                sendLCD(L_L2 + 13, COMMAND);
                printStringLCD(strSp);

                sprintf(strSp, "C:%d", Cooler_status);
                sendLCD(L_L1 + 13, COMMAND);
                printStringLCD(strSp);
            }
        }

        if (DataReceived == 1) {
            DataReceived = 0;
            SP = SerialIn[1];
            TimeH = SerialIn[2];
            TimeL = SerialIn[3];
            CookingTime = (TimeH << 8) + TimeL;
            sendLCD(L_CLR, COMMAND);
            sendLCD(L_L1, COMMAND);
            printStringLCD("Params Set: T ");
            sendLCD(0xdf, DATA);
            sendLCD(L_L2, COMMAND);
            sprintf(strSp, "S:%d T:%lu", SP, CookingTime);
            printStringLCD(strSp);
            startCooking();
        }

        if (CookingInProgress) {
            if (!TargetReached && Temperatura >= SP) {
                TargetReached = 1;
                CookingTime--;
            }

            if (TargetReached && CookingTime > 0) {
                CookingTime--;
                __delay_ms(1000); // Simulate 1-second delay for countdown
            } else if (CookingTime == 0) {
                CookingInProgress = 0;
                TargetReached = 0;
                HEATER = 0;
                COOLER = 0;
                Heater_status = 0;
                Cooler_status = 0;
                sendLCD(L_CLR, COMMAND);
                sendLCD(L_L1, COMMAND);
                printStringLCD("Cooking Done");
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
        }
    }
}

void initializationLCD() {
    TRISD = 0x00;
    TRISE &= ~0x06;

    LCD_RS = 0;
    LCD_EN = 0;
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
    sendLCD(L_NCR, COMMAND);
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
    char i = 0;
    while (s[i] != '\0') {
        sendLCD(s[i++], DATA);
    }
}

void InitPic() {
    INTCON = 0xE0;
    OPTION_REG = 0x03;
    TMR0 = 131;
    TRISA = 0xFF;
}

unsigned int readADC() {
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO != 0);
    unsigned int temp = ((ADRESH << 8) + ADRESL);
    return temp;
}

void initADC() {
    ADCON1 = 0x80;
    ADCON0 = SETLM35;
}

void initUART(unsigned long baudRate) {
    TRISCbits.TRISC7 = 1;
    TRISCbits.TRISC6 = 0;

    TXSTAbits.TXEN = 1;
    RCSTAbits.SPEN = 1;
    RCSTAbits.CREN = 1;
    SPBRG = (char) (_XTAL_FREQ / (64UL * baudRate)) - 1;
    PIE1bits.RCIE = 1;
    INTCONbits.PEIE = 1;
}

void sendUARTString(char * s) {
    char i = 0;
    while (s[i] != '\0') {
        while (!PIR1bits.TXIF);
        TXREG = s[i++];
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
    if (T0IF) {
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
        SerialIn[SerialIndex] = RCREG;
        if (SerialIn[SerialIndex] == '\0') {
            DataReceived = 1;
            SerialIndex = 0;
        } else {
            SerialIndex++;
        }
    }
}

void startCooking(void) {
    CookingInProgress = 1;
    TargetReached = 0;
}
