/**
 * statemachine.h
 * Header for Mealy State Machine Implementation for MSP430G2553 based AI for motorized car
 * Version 1.2.2.5 - 1.2-rc5
 * 
 * Copyright © 2014 Kamilla Productions Uninc.
 * Written by Joonas Greis <joonas.greis@kamillaproductions.com>
 */

#ifndef _STATEMACHINE_H
#define _STATEMACHINE_H

#include "msp430.h"
#include <stdbool.h>

// For the ease of use, so we can disable the interrupts when we are dealing with global vars, otherwise the whole thing would just go BOOOM!

//#define __EINT __enable_interrupt()
//#define __DINT __disable_interrupt()

// Some dummy defines, so the code looks more cool =)
#define CLEAR 0x00
#define EXIT_OK 0

#define SENSOR_LEFT BIT3
#define SENSOR_RIGHT BIT5
#define SENSOR_MID BIT4

#define P1_WHEEL BIT2
#define P2_WHEEL BIT4


static const unsigned short g_kAccelerationTime; // =       2000;           // Time in ms to accelerate from half-speed to maximum

enum eDirection { MID, LEFT, RIGHT };                                   // Directions that car can turn in
extern enum eDirection eLastDir;                                               // Direction that was last turned in
extern enum eDirection eDir;                                                   // Current Direction

extern unsigned short g_u16TurningCounter;                                     // Counter to remove slight direction corrections
extern volatile unsigned short g_u16LongRoadCounter;                                    // Counter to hold how car has gone without turning (for PWM-speed and lap count)
extern unsigned short g_u16SleepCounter;
extern volatile bool SLEEP;
     
/* State Machine declarations */

// State-function declaration are quite self-explanatory don't u think =)
int Init(void);
int StandBy(void);
int Running(void);
int RunningMem(void);
int Exit(void);

// Function pointer for state array typedef declaration
typedef int (*fnptrState)(void);

// Enums for state- and return-codes
enum eStateCode { INIT, STAND_BY, RUNNING, RUNNING_MEM, EXIT};
enum eReturnCode { OK, OK_MEM, ERROR, REPEAT};          // Oh, fuck off.. FAIL already defined in 430g2553.h .. well, lets use ERROR then.

// Declaration for Transition Lookup func.
enum eStateCode LookupTransition(enum eStateCode eState, enum eReturnCode eReturn);



//__interrupt void LongRoadCounter(void);
//__interrupt void SensorInterrupt(void);

#endif //_STATEMACHINE_H