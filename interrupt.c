/**
 * interrupt.c
 * Interrupt functions for MSP430G2553 based AI for motorized car
 * Version 1.1.2.3 - 1.1-rc3
 * 
 * Copyright © 2014 Kamilla Productions Uninc.
 * Written by Joonas Greis <joonas.greis@kamillaproductions.com>
 */

// TODO: REWRITE LONGROADCOUNTER AND REFACTOR AND CLEANUP AND COMMENT EVERYTHING
//       WRITE RUNNING MEMORY FUNCTION

/* Includes */

#include "flipflop.h"
#include "interrupt.h"
#include "statemachine.h"
#include "msp430.h"

/* Functions */

// Sensor Interrupt Service routine, Port 1
#pragma vector = PORT1_VECTOR
__interrupt void SensorInterrupt(void) {
    
    if(P1IFG & BIT3) {          // Rightmost sensor detects black                        
        eDir = RIGHT;           // Set the direction to Right
        P1IFG &= ~BIT3;         // Clear Rightmost sensor Interrupt Flag
        P1IES ^= BIT3;          // Toggle Egde Select
    }
    
    else if(P1IFG & BIT5) {     // Leftmost sensor detects black 
        eDir = LEFT;            // Set the direction to Left
        P1IFG &= ~BIT5;         // Clear Leftmost sensor Interrupt Flag
        P1IES ^= BIT5;          // Toggle Egde Select
    }
    
    else if(P1IFG & BIT4) {     // Middle sensor detects black 
        eDir = MID;             // Set the direction to Middle
        P1IFG &= ~BIT4;         // Clear Middle sensor Interrupt Flag
        P1IES ^= BIT4;          // Toggle Egde Select
    }

}

// Timer1_A0 Interrupt Service routine
#pragma vector=TIMER1_A0_VECTOR 
__interrupt void LongRoadCounter(void) 
{   
    //If direction is changed, reset the Long Road -counter to 0 and PWM duty cycle to half
    if (eLastDir != eDir) {
        
        // Write instructions to FlipFlip Memory
        write_FlipFlop(1, eDir, 3464);
        
        if (g_u16TurningCounter > 500) {
            g_u16TurningCounter = 0;
            g_u16LongRoadCounter = 0;
            eLastDir = eDir;
            TA0CCR1 = g_kAccelerationTime;
            TA1CCR2 = g_kAccelerationTime;
        }
    }
    
    // Turning counter, for slight direction corrections
    if (eDir == LEFT || eDir == RIGHT) {
        g_u16TurningCounter++;
    }
    
    //add speed only if on a straigh
    if (eDir == MID) {
        g_u16TurningCounter = 0;
        g_u16LongRoadCounter++;
        // Set AccelerationPoint, value can be no more than ACCELERATION_TIME
        unsigned short u16AccelerationPoint = g_u16LongRoadCounter;
        if (u16AccelerationPoint > g_kAccelerationTime) u16AccelerationPoint = g_kAccelerationTime;
        //set the PWM duty cycle to wanted value
        //int a = ((double)g_iLongRoadCounter / (double)g_kAccelerationTime) * (g_kTopSpeed >> 1);
        TA0CCR1 = g_kAccelerationTime + u16AccelerationPoint;
        TA1CCR2 = g_kAccelerationTime + u16AccelerationPoint;
    }
    
    // Lap counter, if we stay 4 seconds straight line
    if (g_u16LongRoadCounter > 4000) {
        TA0CCR1 = 0;
        TA1CCR2 = 0;
        SLEEP = true;
        
    }
    if (SLEEP) {
        g_u16SleepCounter++;
    }
    // No more sleep, 1 sec is enough
    if (g_u16SleepCounter > 1000) {
      SLEEP = false;
      g_u16LongRoadCounter = 0;
      g_u16SleepCounter = 0;
    }
} 