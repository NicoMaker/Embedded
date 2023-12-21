#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = ON
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF


#include <xc.h>
#define _XTAL_FREQ 8000000


void initUart(long int);
//void sendUARTString(char *);
char checkString(char *);

//const char * checkString[2][6] = {
//    "giorno",
//    "notte"
//};

const char checkGiorno[6] = "giorno";
const char checkNotte[5] = "notte";
char receivedString[6];
char index;
char endReceive, wrongString;

void main(void) {
    TRISD = 0x00;
    //TRISB = 0x00;
    initUart(9600);

    while (1) {
        if (endReceive) {
            if(checkString(checkGiorno)){
                 PORTD = 0xFF;
            }else if(checkString(checkNotte)){
                 PORTD = 0x00;
            }
            endReceive = 0;
            index = 0;
        }
    }
}

void initUart(long int baudRate) {
    //TRISC = 0x80;
    TRISC |= 0x80; //RC7 forzato a 1
    TRISC &= 0x40; //RC6 forzato a 0
    //    TRISCbits.TRISC7 = 1;
    //    TRISCbits.TRISC6 = 0;

    TXSTA |= 0x20;
    //TXSTAbits.TXEN = 1;
    RCSTA |= 0x90;

    INTCON |= 0xC0;

    PIE1 |= 0x20;
    PIE1bits.RCIE = 1;
    //(long) è il cast a long di 64UL*baudRate
    SPBRG = (_XTAL_FREQ / (long) (64UL * baudRate)) - 1;
}

char checkString(char * c){
    char i = 0;
    while(receivedString[i] != '\0'){
        if(receivedString[i] != c[i]) return 0;
        
        i++;
    }
    return 1;
}

//void sendUARTString(char * s) {
//    char i = 0;
//    while (s[i] != '\0') {
//        while (!PIR1bits.TXIF);
//        TXREG = s[i++];
//    }
//}

void __interrupt() ISR() {
    if (RCIF) {
        RCIF = 0;
        receivedString[index] = RCREG;
        if(receivedString[index] == 13){
            receivedString[index] = '\0';
            endReceive = 1;
        }
        index++;
    }
}
