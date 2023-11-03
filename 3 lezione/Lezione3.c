#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config MCLRE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config CP = OFF


#include <xc.h>
#define _XTAL_FREQ 8000000
#define SEG_A 0x04
#define SEG_B 0x08
#define SEG_C 0x20
#define SEG_D 0x40
#define SEG_E 0x80
#define SEG_F 0x02
#define SEG_G 0x01
#define SEG_P 0x10

//Tutti i segmenti per ogni display
const char display[10] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    SEG_B | SEG_C,
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,
    SEG_B | SEG_C | SEG_F | SEG_G,
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G
};

//Definisci metodo o variabili globalmente
void printMux(char);
void checkButton();
char disp;
char numero;
int countInterrupt;
char start;
char oldRead;

void main(void) {
    //Initial machine setup
    TRISB = 0x00;
    TRISA = 0x02;
    
    //Set TMR0 interrupt registers
    INTCON = 0xA0;
    OPTION_REG = 0x02; //Valore del prescaller
    TMR0 = 6;
    
    numero = 0;
    start = 0;
    //Dividiamo ulteriormente per 256
    //Con XTAL_FREQ a 8MHz, interrupt è chiamato ogni ~ 32ms
    
    while(1){
        checkButton();
        printMux(numero);
    }
}

void checkButton(){
    //char read = ~PORTA & 0x02;//isolo gli altri pulsanti meno RB1
    
    //if(read & !oldRead){
//        __delay_ms(5);
  //      read = ~PORTA & 0x02;
    //    if(read & !oldRead){
      //      start = !start;
        //}
    //}
    //oldRead = read;
    
    char read = ~PORTA & 0x1E;//isolo gli altri pulsanti meno RB1
    
    if((read & 0x02) && !(oldRead & 0x02)){
        __delay_ms(10);
        read = ~PORTA & 0x1E;
        if((read & 0x02) && !(oldRead & 0x02)){
            start = !start;
        }
    }
    
//    if((read & 0x04) && !(oldRead & 0x04)){
//        __delay_ms(10);
//        read = ~PORTA & 0x1E;
//        if((read & 0x04) && !(oldRead & 0x04)){
//            numero = 0;
//        }
//    }
    oldRead = read;
}

//Logica del metodo
void printMux(char n){
    static char unita, decine;
    static char oldNumber;
    
    if(n != oldNumber){
        decine = n / 10;//divisione tra interi => intero
        unita = n % 10;
    }
    
    if(disp){
        PORTB = display[unita] | SEG_P;
    }else{
        PORTB = display[decine];
    }
    oldNumber = n;
}

void __interrupt() ISR(){
    //Per ogni segnale che scatena interrupt,
    //lo gestisco con gli if
    //Non esagerare!!!
    if(T0IF){
        TMR0 = 6;
        T0IF = 0;
        disp = !disp;
        countInterrupt++;
        if(countInterrupt >= 1000){
            if(start){
                numero++;
            }
            countInterrupt = 0;
        }
    }
    
//    if(RCIF){
//        RCIF = 0;
//    }
}