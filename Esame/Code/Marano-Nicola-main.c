#include <xc.h>
#include <stdio.h>

// Configurazione del microcontrollore
#pragma config FOSC = HS        // Oscillatore ad alta velocit?
#pragma config WDTE = OFF       // Watchdog Timer disabilitato
#pragma config PWRTE = ON       // Power-up Timer abilitato
#pragma config BOREN = ON       // Brown-out Reset abilitato
#pragma config LVP = OFF        // Low Voltage Programming disabilitato
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection disabilitato
#pragma config WRT = OFF        // Flash Program Memory Write Protection disabilitato
#pragma config CP = OFF         // Flash Program Memory Code Protection disabilitato

#define _XTAL_FREQ 4000000      // Frequenza del clock

// Definizione dei pin
#define HEATER PORTDbits.RD0
#define COOLER PORTDbits.RD1
#define BUTTON_UP PORTBbits.RB0
#define BUTTON_DOWN PORTBbits.RB1

// Variabili globali
unsigned int setPoint = 370;   // Valore iniziale del setpoint (37.0?C)
unsigned int temperature = 0;  // Temperatura rilevata

void init(void);
void lcd_init(void);
void lcd_command(unsigned char);
void lcd_data(unsigned char);
void lcd_clear(void);
void lcd_set_cursor(unsigned char, unsigned char);
void lcd_print(const char*);
void adc_init(void);
unsigned int adc_read(void);
void uart_init(void);
void uart_transmit(unsigned char);
void uart_send_string(const char*);

void main(void) {
    init();

    lcd_clear();
    lcd_print("Benvenuto!");
    __delay_ms(2000);

    lcd_clear();
    lcd_print("Test");
    __delay_ms(2000);

    while (1) {
        // Legge la temperatura dal sensore LM35
        temperature = adc_read();
        float tempC = temperature * 0.488;

        // Controllo della temperatura con isteresi di 0.5�C
        if (tempC < setPoint / 10.0 - 0.5) {
            HEATER = 1;
            COOLER = 0;
        } else if (tempC > setPoint / 10.0 + 0.5) {
            HEATER = 0;
            COOLER = 1;
        } else {
            HEATER = 0;
            COOLER = 0;
        }

        // Modifica del setpoint tramite pulsanti con debouncing
        if (BUTTON_UP == 1) {
            __delay_ms(20);  // Debouncing
            if (BUTTON_UP == 1) {
                setPoint += 1;
                uart_send_string("SetPoint: ");
                uart_transmit((setPoint / 10) + '0');
                uart_transmit('.');
                uart_transmit((setPoint % 10) + '0');
                uart_send_string(" � C\n");
                __delay_ms(200);
            }
        }
        if (BUTTON_DOWN == 1) {
            __delay_ms(20);  // Debouncing
            if (BUTTON_DOWN == 1) {
                setPoint -= 1;
                uart_send_string("SetPoint: ");
                uart_transmit((setPoint / 10) + '0');
                uart_transmit('.');
                uart_transmit((setPoint % 10) + '0');
                uart_send_string(" � C\n");
                __delay_ms(200);
            }
        }

        // Aggiornamento del display LCD
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("SetPoint: ");
        lcd_data((setPoint / 10) + '0');
        lcd_data('.');
        lcd_data((setPoint % 10) + '0');
        lcd_data('C');

        lcd_set_cursor(1, 0);
        lcd_print("Temp ril: ");
        char buffer[10];
        sprintf(buffer, "%.1f", tempC);
        lcd_print(buffer);
        lcd_data('C');

        __delay_ms(500);
    }
}

void init(void) {
    TRISD = 0x00;  // Configura PORTD come output per HEATER e COOLER
    TRISB = 0xFF;  // Configura PORTB come input per i pulsanti
    TRISC = 0x00;  // Configura PORTC come output per LCD
    adc_init();
    lcd_init();
    uart_init();
}

void lcd_init(void) {
    __delay_ms(20);       // Attendere 20ms dopo l'accensione del display
    lcd_command(0x30);    // Funzione set - inizializza in modalità 8-bit
    __delay_ms(5);        // Attendere 5ms
    lcd_command(0x30);    // Ripeti la funzione set
    __delay_us(200);      // Attendere 200us
    lcd_command(0x30);    // Ripeti la funzione set
    __delay_ms(2);        // Attendere 2ms

    lcd_command(0x38);    // Funzione set - modalità 8-bit, 2 linee, caratteri 5x8
    lcd_command(0x0C);    // Display ON, cursore OFF, blink OFF
    lcd_command(0x06);    // Entry mode - incrementa e non shiftare il display
    lcd_command(0x01);    // Clear display
    __delay_ms(2);        // Attendere 2ms per il clear display
}

void lcd_command(unsigned char cmd) {
    PORTC = cmd;
    PORTDbits.RD2 = 0;  // RS = 0 per inviare un comando
    PORTDbits.RD3 = 1;  // Abilita il display
    __delay_ms(1);
    PORTDbits.RD3 = 0;  // Disabilita il display
}

void lcd_data(unsigned char data) {
    PORTC = data;
    PORTDbits.RD2 = 1;  // RS = 1 per inviare dati
    PORTDbits.RD3 = 1;  // Abilita il display
    __delay_ms(1);
    PORTDbits.RD3 = 0;  // Disabilita il display
}

void lcd_clear(void) {
    lcd_command(0x01);  // Comando per pulire il display
    __delay_ms(2);
}

void lcd_set_cursor(unsigned char row, unsigned char col) {
    unsigned char pos;
    if (row == 0)
        pos = 0x80 + col;
    else
        pos = 0xC0 + col;
    lcd_command(pos);
}

void lcd_print(const char* str) {
    while (*str) {
        lcd_data(*str++);
        __delay_ms(1);  // Aggiungere un breve delay tra i caratteri
    }
}

void adc_init(void) {
    ADCON0 = 0x41;  // Configura il canale 0 e accende il modulo ADC
    ADCON1 = 0x80;  // Configura la giustificazione a destra
}

unsigned int adc_read(void) {
    __delay_ms(2);
    GO_nDONE = 1;          // Avvia la conversione
    while (GO_nDONE);      // Attende la fine della conversione
    return ((ADRESH << 8) + ADRESL);  // Restituisce il valore a 10 bit
}

void uart_init(void) {
    TXSTA = 0x24;  // Configura il trasmettitore UART
    RCSTA = 0x90;  // Configura il ricevitore UART
    SPBRG = 25;    // Imposta il baud rate a 9600 bps (con Fosc = 4MHz)
}

void uart_transmit(unsigned char data) {
    while (!TXIF);  // Attende che il buffer sia vuoto
    TXREG = data;   // Trasmette il dato
}

void uart_send_string(const char* str) {
    while (*str)
        uart_transmit(*str++);
}