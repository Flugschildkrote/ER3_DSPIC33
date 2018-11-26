/* 
 * File:   EnvoiTableaux.h
 * Author: ckokoh
 *
 * Created on 26 novembre 2018, 14:57
 */

#ifndef ENVOITABLEAUX_H
#define	ENVOITABLEAUX_H

#include "Communication.h"

#define TABLEAU_TAILLE 1000
#define ERREUR_TRAME 81
#define PAS_ERREUR_TRAME 80

#ifndef LEDS
    #define LED1 LATBbits.LATB15
    #define LED2 LATBbits.LATB11
    #define LED3 LATBbits.LATB10
#endif

char toASCII(char valeur);
void envoyerTableau(unsigned int *p_tableau);
void attendreDemandeAcquisition(void);
int verifierErreur(void);
void envoyerAck(void);
void EnvoiTableaux(unsigned int *tabSystem, unsigned int *tabConsigne);

#endif	/* ENVOITABLEAUX_H */

