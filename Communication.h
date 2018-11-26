/* 
 * File:   Communication.h
 * Author: abrune26
 *
 * Created on 18 octobre 2018, 08:15
 */

#ifndef COMMUNICATION_H
#define	COMMUNICATION_H

#define CMD_STACK_MAX_SIZE 10
#define DATA_MAX_LEN 0xFF

//#ifndef LEDS
#define LED1 LATBbits.LATB15
#define LED2 LATBbits.LATB11
#define LED3 LATBbits.LATB10
//#endif //LEDS
#define ERROR_LED LED1

#define STX 2
#define CHECKSUM '0'
#define ACK 80
#define RACK 81

#define RS232_NO_ERROR 0
#define RS232_CHECKSUM_ERROR 1
#define RS232_TIMEOUT_ERROR 2

#define MAX_TIMEOUT 2000

enum COMMANDE{
    CMD_RECEPTION_C1 = '1',
    CMD_RECEPTION_C2 = '2',
    CMD_ENVOI_ACK = 16,
    CMD_AQUISITION = 5,
    CMD_REGULATION = 'Y'
};

enum CMD_STATE{
    STATE_STX = 1,
    STATE_LNG = 2,
    STATE_DATA = 3,
    STATE_CHECKSUM = 4
};



void RS232_init(void);

void RS232_setRX(char RX_data);

void RS232_getData(unsigned char *LNGp, char *DATAp, char *errorp);
void RS232_ACK(void);
char RS232_isAvailable(void);
float getValueFromData(unsigned char LNG, char *DATAp);

// Second progamme


// Linker will allocate these buffers from the bottom of DMA RAM.


void initAdc1(void);
void initTmr3(void);
void initDma0(void);
void _DMA0Interrupt(void);

#endif	/* COMMUNICATION_H */

