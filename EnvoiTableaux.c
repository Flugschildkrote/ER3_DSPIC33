#include "EnvoiTableaux.h"
#include "p33FJ64MC802.h"

char toASCII(char valeur){
    if(valeur >= 0 && valeur <= 9){
        valeur += '0';
    }else if(valeur >= 10 && valeur <= 15){
        valeur -= 10;
        valeur += 'A';
    }
    return valeur;
}

void envoyerAck(void){
    while(U1STAbits.UTXBF == 1);
    U1TXREG = '0';
     while(U1STAbits.UTXBF == 1);
    U1TXREG = '2';
     while(U1STAbits.UTXBF == 1);
    U1TXREG = '0';
     while(U1STAbits.UTXBF == 1);
    U1TXREG = '1';
     while(U1STAbits.UTXBF == 1);
    U1TXREG = '8';
     while(U1STAbits.UTXBF == 1);
    U1TXREG = '0';
     while(U1STAbits.UTXBF == 1);
    U1TXREG = '0';
     while(U1STAbits.UTXBF == 1);
    U1TXREG = '1';
}
void envoyerTableau(unsigned int *p_tableau){
    int i, j;
    for(i = 0; i < 11; i++){ // On a 11 trames complètes
        // START
        while(U1STAbits.UTXBF == 1); // Start
        U1TXREG = '0';
         while(U1STAbits.UTXBF == 1); // Start
        U1TXREG = '2';
        
        //LNG
        while(U1STAbits.UTXBF == 1); // LNG
        U1TXREG = 'F';
        while(U1STAbits.UTXBF == 1); // LNG
        U1TXREG = 'F';
        
        char val_checksum=0;
        char val_checksum1, val_checksum2; 
        for(j=0; j < 85; j++){ // Envoi de 85 Cases du tableau (255 octets)
            char valeur1, valeur2, valeur3;
            valeur1 = p_tableau[i*85+j] & 0x00F;
            valeur2 = (p_tableau[i*85+j] & 0x0F0) >> 4; 
            valeur3 = (p_tableau[i*85+j] & 0xF00) >> 8;
            
            valeur1 = toASCII(valeur1); // Convertis le valeur en ASCII
            valeur2 = toASCII(valeur2);
            valeur3 = toASCII(valeur3);
            val_checksum = val_checksum + valeur1 + valeur2 + valeur3;
            while(U1STAbits.UTXBF == 1); // Envoi des valeurs sur le port série
            U1TXREG = valeur3;
            while(U1STAbits.UTXBF == 1);
            U1TXREG = valeur2;
            while(U1STAbits.UTXBF == 1);
            U1TXREG = valeur1;
            
        }
        
        val_checksum = -val_checksum; 
        val_checksum1 =  val_checksum  & 0x00F;
        val_checksum2 = ( val_checksum  & 0x0F0) >> 4;
        val_checksum1 = toASCII(val_checksum1); // Convertis la valeur en ASCII
        val_checksum2 = toASCII(val_checksum2);
        
        // Envoi du CHECKSUM
       
        while(U1STAbits.UTXBF == 1);       
        U1TXREG = val_checksum2;
        while(U1STAbits.UTXBF == 1);
        U1TXREG = val_checksum1;
        //On attend un acknoledge de mathlab
        if(verifierErreur() == ERREUR_TRAME){
            i = i-1;
        }
    }
    
    while(U1STAbits.UTXBF == 1);
    U1TXREG = '0';     
    while(U1STAbits.UTXBF == 1);
    U1TXREG = '2';     
    //LNG
    while(U1STAbits.UTXBF == 1);
    U1TXREG = 'C';
    while(U1STAbits.UTXBF == 1);
    U1TXREG = '3';
     
    do{
        char val_checksum=0;
        char val_checksum1, val_checksum2; 
        for(j=935; j < TABLEAU_TAILLE; j++){
            char valeur1, valeur2, valeur3;
            valeur1 = p_tableau[j] & 0x00F;
            valeur2 = (p_tableau[j] & 0x0F0) >> 4;
            valeur3 = (p_tableau[j] & 0xF00) >> 8;

            valeur1 = toASCII(valeur1);
            valeur2 = toASCII(valeur2);
            valeur3 = toASCII(valeur3);
            val_checksum = val_checksum + valeur1 + valeur2 + valeur3;

            while(U1STAbits.UTXBF == 1);
            U1TXREG = valeur3;
            while(U1STAbits.UTXBF == 1);
            U1TXREG = valeur2;
            while(U1STAbits.UTXBF == 1);
            U1TXREG = valeur1;
        }
          val_checksum = -val_checksum; 
          val_checksum1 =  val_checksum  & 0x00F;
          val_checksum2 = ( val_checksum  & 0x0F0) >> 4;
          val_checksum1 = toASCII(val_checksum1); // Convertis la valeur en ASCII
          val_checksum2 = toASCII(val_checksum2);

         while(U1STAbits.UTXBF == 1);       
         U1TXREG = val_checksum2;
         while(U1STAbits.UTXBF == 1);
         U1TXREG = val_checksum1;
    }while(verifierErreur() == ERREUR_TRAME);
}

int verifierErreur(void){
    char data[255], error;
    unsigned char lng ;
            
    RS232_getData(&lng, data , &error);
    
    return data[0];
}

void EnvoiTableaux(unsigned int *tabSystem, unsigned int *tabConsigne){
    verifierErreur();
    LED1 = 1;  LED2 = 1;  LED3 = 1;
    envoyerTableau(tabSystem);
    LED1 = 1;  LED2 = 1;  LED3 = 0;
    envoyerTableau(tabConsigne);
    LED1 = 0;  LED2 = 0;  LED3 = 1;
}