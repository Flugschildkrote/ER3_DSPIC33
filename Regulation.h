/*
 * File:   Regulation.c
 * Author: amoine03
 *
 * Created on 5 novembre 2018, 14:40
 */

#ifndef REGULATION_H
#define	REGULATION_H

unsigned char flag10ms =0;

// Linker will allocate these buffers from the bottom of DMA RAM.
struct
{
unsigned int Adc1Ch0[1];
unsigned int Adc1Ch1[1];
} BufferA __attribute__((space(dma)));

void initAdc1(void);
void initTmr3(void);
void initDma0(void);
void _DMA0Interrupt(void);
void regulation(void);

#endif	/* REGULATION_H */