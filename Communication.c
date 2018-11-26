#include "Communication.h"
#include "p33FJ64MC802.h"
#include <math.h>

/*enum COMMANDE CMD_Stack[CMD_STACK_MAX_SIZE]; // Stocke les commandes pas encore traitées
unsigned int CMD_StackSize; // Le nombre de commandes non traitées
unsigned int CMD_StackIndex; // La commande suivante à traiter */
enum COMMANDE CMD_Current; // Commande en cours de traitement
enum COMMANDE CMD_Next; // Prochaine commande à traiter
char CMD_Etape = 0; // Étape interne aux commandes

//enum CMD_STATE currentState;

unsigned char LNG;
char DATA[DATA_MAX_LEN]; 
char dataIndex;


//float c1;
//float c2;
static char flag_RX;
static char RX_byte;
unsigned int timeout;

void RS232_init(void){
    timeout = 0;
    flag_RX = 0;
    RX_byte = 0;
}

void RS232_setRX(char RX_data){
    flag_RX = 1;
    RX_byte = RX_data;
}

void RS232_getData(unsigned char *LNGp, char *DATAp, char *errorp){
    char end = 0;
    enum CMD_STATE myState = STATE_STX;
    unsigned int dataIndex = 0;
    char checksum;
    char sum = 0;
    
    while(!end){
        while(!flag_RX){
            if(timeout > MAX_TIMEOUT){  
                *errorp = RS232_TIMEOUT_ERROR; 
                return;
            }
        }
        timeout = 0;
        flag_RX = 0;
        switch(myState){
            case STATE_STX :
                if(RX_byte == STX){
                    myState = STATE_LNG;
                }
                break;
            case STATE_LNG :
                *LNGp = RX_byte;
                myState = STATE_DATA;
                break;
            case STATE_DATA :
                *(DATAp+dataIndex) = RX_byte;
                sum += RX_byte;
                dataIndex ++;
                if(dataIndex == (*LNGp)){
                    myState = STATE_CHECKSUM;
                }
                break;
            case STATE_CHECKSUM :
                checksum = RX_byte;
                if(sum+checksum != 0){
                    *errorp = RS232_CHECKSUM_ERROR;
                }else{
                    *errorp = RS232_NO_ERROR;
                }
                // verifier checksum
                end = 1;
                break;
        }
    }
}

void RS232_ACK(void){
    while(U1STAbits.UTXBF == 1);	// FIFO is full ?
    U1TXREG = '0';
    while(U1STAbits.UTXBF == 1);	// FIFO is full ?
    U1TXREG = '2';
    
    while(U1STAbits.UTXBF == 1);	// FIFO is full ?
    U1TXREG = '0';
    while(U1STAbits.UTXBF == 1);	// FIFO is full ?
    U1TXREG = '1';

    while(U1STAbits.UTXBF == 1);	// FIFO is full ?
    U1TXREG = '8';
    while(U1STAbits.UTXBF == 1);	// FIFO is full ?
    U1TXREG = '0';

    while(U1STAbits.UTXBF == 1);	// FIFO is full ?
    U1TXREG = '0';
    while(U1STAbits.UTXBF == 1);	// FIFO is full ?
    U1TXREG = '0';
}

char RS232_isAvailable(void){ return flag_RX; }

float getValueFromData(unsigned char LNG, char *DATAp){
    float result = 0.0f;
    float power = pow(10, LNG-3);
    int i;
    for(i = 0; i < LNG; i++){
        result += (float)(DATAp[i]-'0')*(float)(power);
            power /= 10.0f;
        }
    return result;
}
