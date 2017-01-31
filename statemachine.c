/**
 * statemachine.c
 * Mealy State Machine Implementation for MSP430G2553 based AI for motorized car
 * Version 1.2.2.5 - 1.2-rc5
 * 
 * Copyright © 2014 Kamilla Productions Uninc.
 * Written by Joonas Greis <joonas.greis@kamillaproductions.com>
 */

/* Includes */

#include "tempsensor.h"
#include "interrupt.h"
#include "statemachine.h"
#include "flipflop.h"
#include "msp430.h"
#include <stdbool.h>

/* Car running globals */

static const unsigned short g_kAccelerationTime = 2000; // Time in ms to accelerate from half-speed to maximum

enum eDirection eLastDir;                               // Direction that was last turned in
enum eDirection eDir;                                   // Current Direction

unsigned short g_u16TurningCounter;                     // Counter to remove slight direction corrections
volatile unsigned short g_u16LongRoadCounter;           // Counter to hold how car has gone without turning (for PWM-speed and lap count)
unsigned short g_u16SleepCounter;
volatile bool SLEEP;

// Transition table for state machine
enum eStateCode eTransition[][4] = { 
	{STAND_BY, STAND_BY, EXIT, INIT},		// init: ok, ok_mem, fail, repeat
	{RUNNING, RUNNING_MEM, EXIT, STAND_BY},		// stand_by: ok, ok_mem, fail, repeat
	{STAND_BY, STAND_BY, EXIT, RUNNING},		// running: ok, ok_mem, fail, repeat
	{STAND_BY, STAND_BY, EXIT, RUNNING_MEM}		// running_mem: ok, ok_mem, fail, repeat
							// no need to define exit_state
};

// Whoa! so many keywords and all this motherfucker does is return enum from enum array
enum eStateCode LookupTransition(enum eStateCode eState, enum eReturnCode eReturn) {
  
    return eTransition[eState][eReturn];
}

// Initialize variables
int Init(void) {
  
    IFG1 = CLEAR;                       // Clear Interrupt Flag
    WDTCTL = WDTPW + WDTHOLD;           // Disable Snoop Dogg Timer

    P1OUT &= CLEAR;                     // Shut down everything!
    P2OUT &= CLEAR;                     // "Why the fuck you keep on running?!? STOP!" *rips all the wiring*

    P1DIR = P1_WHEEL;                       // Set directions (wheels)
    P2DIR = P2_WHEEL;                       // P1.2 & P2.4 OUT

    P1DIR |= BIT0 + BIT6;               // Add some lightshow, onboard LEDs 1.0 & 1.6 OUT 

    P1REN |= SENSOR_LEFT + SENSOR_MID + SENSOR_RIGHT;        // Enable internal pull-up/down resistors for sensors, P1.3, P1.4 & P1.5
    P1OUT |= SENSOR_LEFT + SENSOR_MID + SENSOR_RIGHT;        // Select pull-up mode for P1.3, P1.4 & P1.5
    P1IE |= SENSOR_LEFT + SENSOR_MID + SENSOR_RIGHT;         // Enable interrupt for P1.3, P1.4 & P1.5
    P1IES &= ~(SENSOR_LEFT + SENSOR_MID + SENSOR_RIGHT);     // We start from the Hi/lo edge, toggle when sensor picks up black
    P1IFG &= ~(SENSOR_LEFT + SENSOR_MID + SENSOR_RIGHT);     // Clear IFG from P1.3, P1.4 & P1.5
    __EINT;                             // Enable interrupts __bis_SR_register(GIE)



    //PWM PINS
    //P1SEL |= BIT6;                      // Onboard LED 1.6 for testing
    
    P1SEL |= P1_WHEEL;                      // PWM Pins, aka the wheels
    P2SEL |= P2_WHEEL;                      // P1.2 & P2.4

    //Timer 0 for PWM
    TA0CCR0 = g_kAccelerationTime << 1; // PWM Period
    TA0CCTL1 = OUTMOD_7;                // CCR1 PWM output mode: 7 - PWM reset/set
    //TA0CCTL2 = OUTMOD_7;                // CCR2 PWM output mode: 7 - PWM reset/set
    TA0CCR1 = g_kAccelerationTime;      // CCR1 PWM duty cycle
    //TA0CCR2 = g_kAccelerationTime;      // CCR2 PWM duty cycle
    TA0CTL = TASSEL_2 + MC_1;           // TACLK = SMCLK, 1MHz, Up to CCR0.

    //Timer 1 for PWM
    TA1CCR0 = g_kAccelerationTime << 1; // PWM Period 
    TA1CCTL2 = OUTMOD_7; // TACCR2 reset/set 
    TA1CCR2 = g_kAccelerationTime; // TACCR2 PWM duty cycle 
    
    //Timer 1 for long straight counter and for saving the track in memory
    TA1CCR0 = 1000;                     // 1MHz / (ID_N * CCR0) = 1000Hz = 1ms
    TA1CCTL0 |= CCIE;                   // Compare-mode interrupt. (This should be activated initiating running-state, its unnecessary before that
    TA1CTL = TASSEL_2 + MC_1;           // TACLK = SMCLK, 1MHz, Up to CCR0.
    //TA1CCTL0 &= ~CCIE;                // Disable timer Interrupt
    
    
    g_u16LongRoadCounter = 0;           // Reset the long straight counter, pointless unless Timer 1 compare mode is set in running state init
    
    SLEEP = false;                      // We don't want to sleep
    g_u16SleepCounter = 0;              // Reset the sleep counter
    
    //__bis_SR_register(SCG0 + SCG1 + OSCOFF);
    
    InitTemperatureSensor();
    
    return OK;                          // Annie are u OK?
}

// Stand By state, waiting for someone to heat up the microcontroller, LOL what a start-up-mechanism
int StandBy(void) {
  
    //Checking if current temperature and environment temperature difference is greater than 2C
    if (GetTemperature() < g_u16EnvTemp + 1)    // Set 3-5 for RELEASE, testing mode 1 to skip warming up
        return REPEAT;                  // Repeat state until the chip has warmed up
    else return OK;                     // OK, Lets put some music on!
    
    //ADC10CTL0 |= ENC + ADC10SC;
    //__bis_SR_register(CPUOFF + SCG0 + SCG1 + OSCOFF);
    //if (Degree < g_u16EnvTemp + 3)
    //    return REPEAT;
    //else return OK;
    
}

// Its alive!!! Run Forrest, run! ..oh, wrong movie. :c
int Running(void) {
  
    // Disable interrupts so we don't fuck up by using same vars simultaneously
    //__DINT;
  
    if (!SLEEP) {
  
        switch(eDir) {
          
        case LEFT:
          TA0CCR1 = 0;
          TA1CCR2 = g_kAccelerationTime;
          //P1OUT &= ~P1_WHEEL;                   // Big wheel keep on turnin'
          //P2OUT |= P2_WHEEL;
          break;
          
        case RIGHT:
          TA0CCR1 = g_kAccelerationTime;
          TA1CCR2 = 0;
          //P1OUT |= P1_WHEEL;                    // Proud Mary keep on burnin'
          //P2OUT &= ~P2_WHEEL;
          break;
          
        case MID:
          TA0CCR1 = g_kAccelerationTime + g_u16LongRoadCounter;
          TA1CCR2 = g_kAccelerationTime + g_u16LongRoadCounter;
          //P1OUT |= P1_WHEEL;                    // Rollin', rollin', rollin' on the river
          //P2OUT |= P2_WHEEL;
          break;
          
        default:
          // Oh lol, did someone forgot to write something here, eh?
          break;
          
        }
        
    }
    
    // Don't forget to enable interrupts :>
    //__EINT;

    return REPEAT;
}

// Lets play a memory game
int RunningMem(void) {
  
    return OK;
}

// Lets get the fuck out of here!
int Exit(void) {
  
    return OK;
}




//#pragma vector=ADC10_VECTOR                     // ADC10 interrupt service routine  //+ ADC10IE;  
//__interrupt void ADC10_ISR (void) {
//      temp = ADC10MEM;                            // Read ADC sample
//      Degree = (temp * 27069L - 18169625L) >> 16;
//      ADC10CTL0 &= ~ENC;
//      __bic_SR_register_on_exit(CPUOFF + SCG0 + SCG1 + OSCOFF);                           // Exit low-power mode
//}
