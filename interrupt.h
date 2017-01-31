/**
 * interrupt.h
 * Header for Interrupt Functions for MSP430G2553 based AI for motorized car
 * Version 1.0.2.0 - 1.0-rc0
 * 
 * Copyright © 2014 Kamilla Productions Uninc.
 * Written by Joonas Greis <joonas.greis@kamillaproductions.com>
 */

#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "msp430.h"

// For the ease of use, so we can disable the interrupts when we are dealing with global vars, otherwise the whole thing would just go BOOOM!

#define __EINT __enable_interrupt()
#define __DINT __disable_interrupt()

/* Interrupt function declarations */

__interrupt void LongRoadCounter(void);
__interrupt void SensorInterrupt(void);

#endif //_INTERRUPT_H