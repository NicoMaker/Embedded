void initUART(long int baudRate) //ABILITA COMUNICAZIONE SERIALE
{
   //QUESTO METODO TASSA LA MACCHINA
    //TRISCbits.TRISC7 = 1; //bit 7 di trisC settato a 1 --> ABILITA SERIALE
    //TRISCbits.TRISC6 = 0; //bit 6 di trisC settato a 0 --> ABILITA SERIALE
    
    //ALTRO MODO DI FARLO, MENO TASSANTE PER LA MACCHINA --> avviene comparando il registro con se stesso (?))
    TRISC |= 0x80; // inserisco il bit 7 a 1 --> 1000 0000 --> faccio una comparazione OR bit per bit e ottengo 1000 0000
    TRISC &= ~0x40; // inserisco il bit 6 a 1 e nego tutto --> 1011 1111 e faccio una comparazione AND tra 1000 0000 e la sua negazione (~) --> 1011 1111 = 1000 0000

    
    //registro di configurazione della trasmissione 
    
    //TXSTAbits.TXEN =1;  //bit 5 di TXSTA a 1 --> ABILITA TRASMISSIONE
    TXSTA |= 0X20;
    
    //con i bits devo usare più righe e tassare la macchina, con la comparazione OR me ne basta 1
    //RCSTAbits.CREN = 1;
    //RCSTAbits.SPEN = 1;
    RCSTA |= 0x90;
    
    INTCON |= 0xC0; // --> abilito bit 6(interrupt) e 7(trasmissione dei dati)
    PIE1 |= 0x20; // --> abilito bit 5 (interrupt)

    //imposto la frequenza a cui verrano trasmessi i dati
    SPBRG = (_XTAL_FREQ / (long) (64UL * baudRate)) -1;  //divido la frequenza per il cast a long di un Unsigned Long(senza segno) a 64 bit * la baudRate e poi meno 1
}


//MANDA STRINGA A TERMINALE TRAMITE SERIALE
void sendUARTString (char * s) //il puntatore va a puntare una zona di memoria dove è presente un array di char
{
    char i = 0;
    while(s[i] != '\0') //fintanto che il char che stai puntando è diverso da \0 (ovvero la fine dell'array) fai:
    {
        while(!PIR1bits.TXIF); //fintanto che sta trasmettendo il dato resterà su questa istruzione
        TXREG = s[i++]; //esce dal ciclo, assegna, incrementa, ricomincia
    }
}

//ASSEGNA UN ID AI PULSANTI DEL TASTIERINO
char buttonMatrix() //consente il funzionamento del tastierino numerico, essenzialmente lo inizializza e da un ID a ogni pulsante
{
    //backup delle impostazioni di TRISB e TRISD
    char trisB_bku = TRISB;
    char trisD_bku = TRISD;
    
    char k = 99; //valore diverso dal range in cui stanno il contenuto dei pulsanti
    
    //REGISTRO TRIS --> 1 = INPUT ; 0 = OUTPUT
    TRISD = 0X0F; //REGISTRO DELLE RIGHE DEL TASTIERINO (INPUT) --> 0000 1111, quindi RD0, 1, 2, 3 attivati(quelli che interessano il tastierino)
    TRISB &= ~0X07; //REGISTRO DELLE COLONNE DEL TASTIERINO (OUTPUT) --> 1111 0000 (nego quello sopra), quindi RB0, 1, 2
    
    for (char col = 0; col <3; col++) //scorre sulle colonne quindi B
    {
        PORTB = columnConfig[col]; //imposta PORTB sul valore attuale dell'array sopra
        __delay_us(50); //delay per rilevare il button press
        
        char read = ~PORTD & TRISD; // 0000 0000 & 0000 1111 = 0000 0000, premo un pulsante e il led in PORTD si spegne
        
        for (char row = 0; row < 4; row++) //scorre sulle righe quindi D
        {
            if(read & (1 << row)) //comparo in AND read e shifto a sinistra per il valore di row, se è TRUE faccio: 
            {
                k = row + col * 4; // se non entra mai qui, k = 99 e quindi nessun pulsante premuto, altrimenti assegno a k il valore dell' ID del pulsante  premuto
            }
        }
    }
    TRISB = trisB_bku;
    TRISD = trisD_bku;
    
    return k;
}


//INVIA STRINGA AL LCD
void sendLCDString(char * s)
{
    char i = 0;
    while(s[i] != '\0') //fintanto che i è diverso da \0
    {
        sendLCD(s[i], DATA); //inviai char su index i ed incrementa, setta DATA su rs di sendLCD
        i++;
    }
}


//COMANDO ESSENZIALE PER MANDARE STRINGA AL LCD
void sendLCD(char dato, char rs) //STEP NECESSARI PER MANDARE DATI O COMANDI SU LCD IN BASE AL VALORE CHE OTTIENE RS
{
    LCD_EN = 1;
    LCD_PORT = dato;
    LCD_RS = rs;
    __delay_ms(3);

    LCD_EN = 0;
    __delay_ms(3);

    LCD_EN = 1;
}


 //STEP NECESSARI PER INIZIALIZZARE LCD, SONO SEMPRE QUELLI
void initLCD()
{
    LCD_TRIS = 0x00;
    TRISE &= ~0x06;

    LCD_EN = 0;
    LCD_RS = 0;

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
    sendLCD(L_CUR, COMMAND);
    sendLCD(L_L1, COMMAND);
}