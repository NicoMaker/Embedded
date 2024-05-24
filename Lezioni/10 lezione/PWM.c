// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define _XTAL_FREQ 8000000
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

#include <xc.h>

void sendLCD(char , char);
void initLCD(void);
void printLCD(char *);
void initPic(void);
void initADC(void);
int readADC(void);
void convert(int , char* );
char NumberLenght(long);
char readTemp(void);
void setPoint(char *);
void setDC(char);

char writeLCD = 1;
char DutyCycle;
char DCCooler;

void main(void) {
    initPic();
    initLCD();
    initADC();
    printLCD("Test");
    __delay_ms(2000);
    CLEAR_DISPLAY;
    char stringTemp[16];
    char stringSP[16];
    
    char temp = 0;
    char setpoint = 50;
    
    while(1)
    {
        temp = readTemp();
        setPoint(&setpoint);
        
        DutyCycle = 90;
        DCCooler = 73;
       
        
        if (writeLCD)
        {
            convert(temp, stringTemp);
            sendLCD(L_L1, COMMAND);
            printLCD("Temp: ");
            printLCD(stringTemp);
            convert(setpoint, stringSP);
            sendLCD(L_L2, COMMAND);
            printLCD("SetPoint: ");
            printLCD(stringSP);
            writeLCD = 0;
        }     
    }
    return;
}

void setPoint(char *sP)
{
    static char oldB;
    char read = ~PORTB & 0x03;
    if ((read & 0x01) && !(oldB & 0x01))
    {
        (*sP)--;
        writeLCD = 1;
    }
    if ((read & 0x02) && !(oldB & 0x02))
    {
        (*sP)++;
        writeLCD = 1;
    }
    oldB = read & 0x03;    
}

char readTemp()
{
    char temp;
    static int oldVal;
    ADCON0 = 0x11;
    int val = readADC();
    if (val != oldVal)
    {
        temp = (char)(val * 0.488);
        oldVal = val; 
        writeLCD = 1;
    }

    return temp;
}

int readADC()
{
    ADCON0bits.GO = 1;
    while(ADCON0bits.GO != 0);        
    return ADRESL + (ADRESH << 8);
}

void initADC()
{
    TRISA = 0xFF;
    ADCON0 = 0x11;
    ADCON1 = 0x89;
    __delay_us(20);    
}

void initPic()
{
    TRISA = 0xFF; // imposto tutto input
    TRISB = 0x03; // RB0 e RB1 come input, il resto output
    TRISC = 0x00; // tutti output
    TRISD = 0x00; // pilotaggio LCD Output
    TRISE = 0x00; // pilotaggio LCD output
    INTCON = 0xA0; // abilito GIE e TOIE
    OPTION_REG = 0x08;
    TMR0 = 6;
}

void printLCD(char s[])
{
    int i = 0;
    while(s[i] != '\0')
    {
        sendLCD(s[i++], DATA);
    }
}

void initLCD()
{
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
    sendLCD(L_CUR, COMMAND); // 0x0E curso on, no blink
    sendLCD(L_L1, COMMAND);
}

void sendLCD(char dato, char rs)
{
    LCDEN = 1;
    LCDPORT = dato;
    LCDRS = rs;
    __delay_ms(3);
    LCDEN = 0;
    __delay_ms(3);
    LCDEN = 1;
}

 void convert(int num, char* s)
 {
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
        s[i+1] = '\0';
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

 
 void __interrupt() ISR()
 {
     static char count;
     if (T0IF)
     {
         T0IF = 0;
         TMR0 = 6;
         //PORTBbits.RB7 ^= 1;
         count++;
         if (count > 100) count = 0;
         
         if (DutyCycle > count) HEATER = 1;
         else HEATER = 0;
         
         if (DCCooler > count) COOLER = 1;
         else COOLER = 0;
     }
 }