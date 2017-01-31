/**
 * tempsensor.h
 * Header for Temperature sensor Functions for MSP430G2553 based AI for motorized car
 * Version 1.0.2.0 - 1.0-rc0
 * 
 * Copyright © 2014 Kamilla Productions Uninc.
 * Written by Joonas Greis <joonas.greis@kamillaproductions.com>
 */

#ifndef _TEMPSENSOR_H
#define _TEMPSENSOR_H

/* Temperature measurement declarations and globals */

// Quite self-explanatory
void InitTemperatureSensor(void);
unsigned short GetTemperature(void);

// Enviroment Temperature to compare heating
static unsigned short g_u16EnvTemp;

#endif //_TEMPSENSOR_H