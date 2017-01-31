/**
 * main.c file for MSP430G2553 based AI for motorized car
 * 
 * Copyright © 2014 Kamilla Productions Uninc.
 * Written by Joonas Greis <joonas.greis@kamillaproductions.com>
 */

#include "statemachine.h" 
#include "msp430.h" 
     
fnptrState fnrgState[5] = {Init, StandBy, Running, RunningMem, Exit};

int main(int argc, char *argv[]) {

    enum eStateCode eState = INIT;      // Setting the initial stage
    enum eReturnCode eReturn;           // Defining the return-code variable
    int (* fnState)(void);              // Defining function pointer to state-functions

    for (;;) {                          // Forever alone-loop ;_;
        fnState = fnrgState[eState];    // Getting function call for our current state 
        eReturn = (enum eReturnCode)fnState();  // Call the function and get the return code
        if (EXIT == eState)             // If we hit the exit-state, shut the fuck down
            break;
        eState = LookupTransition(eState, eReturn);     // Get next state accourding to current state and return code by looking from the transition table
    }

    return EXIT_OK;                     // Everything is OK, GTFO!
}
