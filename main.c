//###################################################################
//###########################[INCLUDE]###############################
//###################################################################

#include "p33FJ64MC802.h"
#include "Communication.h"
#include "EnvoiTableaux.h"

//###################################################################
//########################[CONSTANTES]###############################
//###################################################################

#define FRC 7370000 //see datasheet Page 146 

_FOSC(FCKSM_CSDCMD & OSCIOFNC_ON & POSCMD_NONE);
_FOSCSEL(FNOSC_FRCPLL);					//Oscillator FRC with Selection PLL
//_FOSCSEL(FNOSC_FRC);					//Oscillator FRC without PLL
																	
_FWDT(FWDTEN_OFF);					//Turn off WatchDog Timer
_FGS(GCP_OFF);						//Turn off code protect
_FPOR(FPWRT_PWR1);					//Turn off power up timer

#define PERIOD 49517					// sets the default interval flash rate
#define FLASH_RATE 150						// smaller value yields faster rate (1-100)
#define FOREVER 1							// endless 

#define NULL 0

#ifndef LEDS
    #define LED1 LATBbits.LATB15
    #define LED2 LATBbits.LATB11
    #define LED3 LATBbits.LATB10
#endif

//###################################################################
//###################[FONCTIONS-VARIABLES]###########################
//###################################################################

void Setup();
void InitTimer1();	
void Wait_CMD(void);
void Rack(void);

//#####################[GLOBALES]##########################
unsigned int Counter = 0;
volatile unsigned int timer_expired;
unsigned char flag10ms =0;


//#####################[REGULATION]##########################
int Commande = 0x14A;
int Commande0;
unsigned int Consigne, Sortie;
int epsilon0 ;
struct{
    unsigned int Adc1Ch0[1];
    unsigned int Adc1Ch1[1];
} BufferA __attribute__((space(dma)));

void Regulation(void);
void EnvoiCNA(int Commande);

//##################[ENVOI TABLEAU]##########################

unsigned int Entree_tab[TABLEAU_TAILLE];
unsigned int Sortie_tab[TABLEAU_TAILLE];
unsigned int *tableau1, *tableau2;

//##################[RECEPTION PARAMETRES]####################
float c1, c2;
float *parameterPointer = 0;
void Changement_Parametre(void);
void Demande_Aquisition(void);

//##################[VARIABLES MACHINE ETATS]##########################
enum STATE{
    STATE_WAIT_CMD,
    STATE_RACK,
    STATE_CHANGEMENT_PARAMETRE,
    STATE_DEMANDE_AQUISITION,
    STATE_REGULATION
};
enum STATE currentState;


//###################################################################
//####################[DEBUT POGRAMME]###############################
//###################################################################

int main ( void ){
    
    //Initialisation
    Setup();
    
    EnvoiCNA(0x200);
    
    unsigned int i;
    for(i = 0; i < TABLEAU_TAILLE; i++){
        Entree_tab[i] = i;
        Sortie_tab[i] = (999-i);
    }
    
    tableau1 = Entree_tab;
    tableau2 = Sortie_tab;

	while (FOREVER)
	{
        switch(currentState){
            case STATE_WAIT_CMD :
                Wait_CMD();
                break;
            case STATE_RACK : 
                Rack();
                break;
            case STATE_CHANGEMENT_PARAMETRE :
                Changement_Parametre();
                break;
            case STATE_DEMANDE_AQUISITION : 
                Demande_Aquisition();
                break;
            case STATE_REGULATION :
                Regulation();
                break;
        }
	}
}
void Setup(void){
    /*Initialize ports */
	LATB  = 0x0000; 				// set latch levels
	TRISB = 0x0000; 				// set IO as outputs
    
    //Map output
    RPOR6 = 0b0000100000000111;   //Clock : 01000    Data : 00111
    //RPOR7 = 0b0000001001; //SPI Slave Select Output on pin 14
    
    //Initialize SPI config
    IFS0bits.SPI1IF = 0; //Clear the Interrupt Flag
    IEC0bits.SPI1IE = 0; //disable the Interrupt
    // SPI1CON1 Register Settings
    SPI1CON1bits.DISSCK = 0; //Internal Serial Clock is Enabled.
    SPI1CON1bits.DISSDO = 0; //SDOx pin is controlled by the module.
    SPI1CON1bits.MODE16 = 1; //Communication is word-wide (16 bits).
    SPI1CON1bits.SMP = 0; //Input Data is sampled at the middle of data output time.
    SPI1CON1bits.CKE = 0; //Serial output data changes on transition from
    
    //Idle clock state to active clock state
    SPI1CON1bits.CKP = 0; //Idle state for clock is a low level;
    
    //active state is a high level
    SPI1CON1bits.MSTEN = 1; //Master Mode Enabled
    SPI1STATbits.SPIEN = 1; //Enable SPI Module
    
    
    
    //PLL setup: Datasheet Page :  147
	CLKDIVbits.PLLPRE = 0;          // N1=2: PLL VCO Output Divider Select bits; 0 -> /2 (default)
	PLLFBDbits.PLLDIV = 43 - 2;  	// M=42: PLL Feedback Divisor bits;(divisor is 2 more than the value)
	CLKDIVbits.PLLPOST = 0;         // N2=2: PLL Phase Detector Input Divider bits; 0 -> /2
	
    //FOS = FRC * M/(N1*N2)         // FRC=7.37MHz //see datasheet Page 146
	//Fcy = FOS/2= 39092500  //38.6935MHz
    
	while(OSCCONbits.LOCK != 1);	// Wait for PLL to lock
	RCONbits.SWDTEN = 0;      // Disable Watch Dog Timer

	RPINR18bits.U1RXR = 7; //Map U1RX to RP7 pin 16
	RPOR3bits.RP6R = 3; //Map U1TX to RP6 pin 15
	U1BRG = 125; //19200
//	IFS0bits.U1RXIF = 0; //reset U1RX interrupt flag 
	IEC0bits.U1RXIE = 1; //Enable U1RX Interrupt
	U1MODEbits.UARTEN = 1;	// And turn the peripheral on
	U1STAbits.UTXEN = 1;

	/* 	Initialize ports */
	LATA  = 0x0000; 				// set latch levels
	TRISA = 0x0003; 				// set IO as outputs
	LATB  = 0x0000; 				// set latch levels
	TRISB = 0x0080; // set IO as outputs Rb7 as input
	
	InitTimer1();					// start the timer
	
    LED1 = 0;
    LED2 = 0;
    LED3 = 0;
    
    currentState = STATE_WAIT_CMD;
    RS232_init();
    
    //Regulation
    initDma0();
    initAdc1();
    initTmr3();
}

// A completer
void Wait_CMD(void){
    while(RS232_isAvailable() == 0){
        // Sortie CNA à 2.5V
    }
    
    unsigned char LNG;
    char DATA[DATA_MAX_LEN];
    char ERROR;
    RS232_getData(&LNG, DATA, &ERROR);
    if(ERROR != 0 || LNG != 1){
        ERROR_LED = 1;
    }
    
    switch(DATA[0]){
        case CMD_RECEPTION_C1 :
            parameterPointer = &c1;
            currentState = STATE_CHANGEMENT_PARAMETRE;
            break;
        case CMD_RECEPTION_C2 :
            parameterPointer = &c2;
            currentState = STATE_CHANGEMENT_PARAMETRE;
            break;
        case CMD_ENVOI_ACK :
            currentState = STATE_RACK;
            break;
        case CMD_AQUISITION :
            currentState = STATE_DEMANDE_AQUISITION;
            break;
        case CMD_REGULATION :
            currentState = STATE_REGULATION;
            break;
        default :
            ERROR_LED = 1;
            break;
    }
    
    RS232_ACK();
    
    // On traite la demande
}

void Rack(void){
    currentState = STATE_WAIT_CMD;
}
// A completer
void Changement_Parametre(void){
    while(RS232_isAvailable() == 0){
        // Sortie CNA à 2.5V
    }
     
    unsigned char LNG;
    char DATA[DATA_MAX_LEN];
    char ERROR;
    RS232_getData(&LNG, DATA, &ERROR);
    if(ERROR != RS232_NO_ERROR || LNG == 0){
        ERROR_LED = 1;
    }
    
    (*parameterPointer) = getValueFromData(LNG, DATA);
    RS232_ACK();
    currentState = STATE_WAIT_CMD;
}
// A completer
void Demande_Aquisition(void){
    Counter=0;
    EnvoiCNA(0x0DE);          // echelon = 1 V
    while(Counter<100){} // att 1s
    EnvoiCNA(0x14D);          // echelon = 1,5 V
    while(Counter<300){} // att 2s
    EnvoiCNA(0x0DE);          // echelon = 1 V
    while(Counter<500){} // att 2s*/

    EnvoiTableaux(tableau1, tableau2);
    
    currentState = STATE_WAIT_CMD;
}
// A completer
void Regulation(void){
    int epsilon ;
    
    while(RS232_isAvailable() == 0){
        while(flag10ms == 0);
        epsilon = Consigne - Sortie;                                //calcul du signal d'erreur
        Commande = (c1 * epsilon) + (c2 * epsilon0 ) + (Commande0); //calcul du signal de commande
        if (Commande>0x3FF) Commande = 0x3FF;                       //saturation haute
        if (Commande<0) Commande = 0;                               //saturation basse
     
        EnvoiCNA(Commande);
        
        epsilon0 = epsilon ;                                        //l'actuel signal d'erreur devient l'ancien
        Commande0 = Commande;
        flag10ms = 0;
    }
    currentState = STATE_WAIT_CMD;
}

/*---------------------------------------------------------------------
  Function Name: InitTimer1
  Description:   Initialize Timer1 for 1 second intervals
-----------------------------------------------------------------------*/
void InitTimer1( void ){
	T1CON = 0;						/* ensure Timer 1 is in reset state */
 	IFS0bits.T1IF = 0;				/* reset Timer 1 interrupt flag */
	IPC0bits.T1IP = 4;				/* set Timer1 interrupt priority level to 4 */
 	IEC0bits.T1IE = 1;				/* enable Timer 1 interrupt */
	PR1 = PERIOD;					/* set Timer 1 period register */
	T1CONbits.TCKPS1 = 0;			/* select Timer1 Input Clock Prescale */
   	T1CONbits.TCKPS0 = 1;			/* select Timer1 Input Clock Prescale */

	T1CONbits.TCS = 0;			 	/* select external timer clock */
	T1CONbits.TON = 1;			 	/* enable Timer 1 and start the count */ 
}

/*---------------------------------------------------------------------
	Interrupt: _T1Interrupt TMR1 Timer 1 expired 
-----------------------------------------------------------------------*/
void __attribute__((interrupt, auto_psv)) _T1Interrupt( void ){
	timer_expired = 1;				/* flag */
	Counter++;						/* keep a running counter */
    flag10ms = 1;
 	IFS0bits.T1IF = 0;				/* reset timer interrupt flag	*/
}	

/*---------------------------------------------------------------------
	Interrupt:_U1RXInterrupt UART1RX Uart 1 Receiver 
-----------------------------------------------------------------------*/
void __attribute__((interrupt, auto_psv)) _U1RXInterrupt ( void ){

    RS232_setRX(U1RXREG);
	IFS0bits.U1RXIF = 0;  //reset U1RX interrupt flag
   // LED3 ^= 1;
}	

/*---------------------------------------------------------------------
	Interrupt: _U1TXInterrupt  UART1TX Uart 1 Transmitter 
-----------------------------------------------------------------------*/
void __attribute__((interrupt, auto_psv)) _U1TXInterrupt( void ){

}

/*========================================================================================
_DMA0Interrupt(): ISR name is chosen from the device linker script.
========================================================================================*/
void __attribute__((__interrupt__, auto_psv)) _DMA0Interrupt(void){
    Consigne = BufferA.Adc1Ch0[0];
    Sortie = BufferA.Adc1Ch1[0];
    IFS0bits.DMA0IF = 0; //Clear the DMA0 Interrupt Flag
}

/*======================================================================================
Timer 3 is setup to time-out every 125 microseconds (8Khz Rate). As a result, the module
will stop sampling and trigger a conversion on every Timer3 time-out, i.e., Ts=125us.
=======================================================================================*/
void initTmr3(){
    TMR3 = 0x0000;
    PR3 = 4999; // Trigger ADC1 every 125usec
    IFS0bits.T3IF = 0; // Clear Timer 3 interrupt
    IEC0bits.T3IE = 0; // Disable Timer 3 interrupt
    T3CONbits.TON = 1; //Start Timer 3
}


/*==========================================================================================
ADC Initialization for Channel Scan
===========================================================================================*/
void initAdc1(void){
    AD1CON1bits.FORM = 0; // Data Output Format: Integer
    AD1CON1bits.SSRC = 2; // Sample Clock Source: GP Timer starts conversion
    AD1CON1bits.ASAM = 1; // ADC Sample Control: Sampling begins immediately after conversion
    AD1CON1bits.AD12B = 0; // 10-bit ADC operation
    AD1CON1bits.SIMSAM = 1; // Samples CH0, CH1, CH2, CH3 simultaneously (when CHPS<1:0> = 1x)
    AD1CON2bits.BUFM = 0;
    AD1CON2bits.CSCNA = 1; // Scan Input Selections for CH0+ during Sample A bit
    AD1CON2bits.CHPS = 1; // Converts CH0 and CH1
    AD1CON2bits.VCFG = 0;
    AD1CON3bits.ADRC = 0; // ADC Clock is derived from Systems Clock
    AD1CON3bits.ADCS = 63; // ADC Conversion Clock
    AD1CHS0bits.CH0SA = 0; // MUXA +ve input selection (AIN0) for CH0
    AD1CHS0bits.CH0NA = 0; // MUXA -ve input selection (Vref-) for CH0
    AD1CHS123bits.CH123SA = 0; // MUXA +ve input selection (AIN0) for CH1
    AD1CHS123bits.CH123NA = 0; // MUXA -ve input selection (Vref-) for CH1
    AD1CHS123bits.CH123SB = 0;
    AD1CSSL = 0x0003; // Scan AIN0, AIN1 inputs
    AD1CON1bits.ADDMABM = 0; // DMA buffers are built in scatter/gather mode
    AD1CON2bits.SMPI = 3; // 4 ADC buffers
    AD1CON4bits.DMABL = 3; // Each buffer contains 8 words
    IFS0bits.AD1IF = 0; // Clear the A/D interrupt flag bit
    IEC0bits.AD1IE = 0; // Do Not Enable A/D interrupt
    AD1CON1bits.ADON = 1; // Turn on the A/D converter
}
void initDma0(void){
    DMA0CONbits.AMODE = 2; // Configure DMA for Peripheral indirect mode
    DMA0CONbits.MODE = 2; // Configure DMA for Continuous Ping-Pong mode
    DMA0PAD = 0x0300; // Point DMA to ADC1BUF0
    DMA0CNT = 3; // 4 DMA request (2 buffers, each with 1 word)
    DMA0REQ = 13; // Select ADC1 as DMA Request source
    DMA0STA = __builtin_dmaoffset(&BufferA);//We use just PrimaryMemory
    IFS0bits.DMA0IF = 0; //Clear the DMA interrupt flag bit
    IEC0bits.DMA0IE = 1; //Set the DMA interrupt enable bit
    DMA0CONbits.CHEN=1; // Enable DMA
}
void EnvoiCNA(int valeur_a_convertir){
    int i;
    while(SPI1STATbits.SPITBF == 1) {}
        LATBbits.LATB14 = 1;
        SPI1BUF = valeur_a_convertir; //envoi commande
        for(i=0;i<1500;i++) {
            //Attente avant prochain envoi
        }
    LATBbits.LATB14 = 0;
}