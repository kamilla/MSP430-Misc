/**
 * tempsensor.c
 * Temperature sensor functions for MSP430G2553 based AI for motorized car
 * Version 1.0.2.1 - 1.0-rc1
 *
 * From datasheet we have these formulas:
 * 
 * VSensor,typ = TCSensor T [°C] + VSensor(TA = 0°C) [mV]
 * -> VSensor = TCSensor * Temp + VSensor(0°C) <-> Temp = (VSensor - VSensor(0°C)) / TCSensor
 * 
 * N_ADC = 1023 * (VSensor - VRef-) / (VRef+ - VRef-)
 * with VRef- = 0 (using VRef+ = 1.5V) -> N_ADC = 1023 * VSensor / VRef+ <-> VSensor = N_ADC * VRef+ / 1023
 * 
 * -> Temp = ((N_ADC * VRef+ / 1023) - VSensor(0°C)) / TCSensor
 * 
 * 
 * MSP430x2xx Family User's Guide:
 * 
 *              VSensor(0°C) = 986mV
 * 
 * and from MSP430G2553 datasheet:
 *
 *              TCSensor (Vcc = 3 V) = 3.55 mV/°C
 * 
 * 
 * we also choose ADC10CTL0 = SREF_1 so we get
 * 
 *              Vref+ = 1.5V
 *              Vref- = 0V
 * 
 * 
 * -> Temp = ((N_ADC * 1500mV / 1023) - 986mV) / (3.55 mV/°C)
 * -> Temp = (150000N_ADC - 100867800) / 363165
 * -> Temp = (422,94824666473916814671017306183N_ADC - 284412,39436619718309859154929577) / 2^10
 * 
 * and with a little approximation and rounding we get
 * 
 * -> Temp = 423N_ADC - 284412) / 2^10 -> Temp = (423 * N_ADC - 284412) >> 10
 * 
 *
 * Copyright © 2014 Kamilla Productions Uninc.
 * Written by Joonas Greis <joonas.greis@kamillaproductions.com>
 */

/* Includes */

#include "msp430.h"
#include "tempsensor.h"

/* Functions */
     
/**
 * Function InitTemperatureSensor()
 *
 * Initializes the temperature sensor and sets Analog-Digital Conversion values
 */
void InitTemperatureSensor(void) {
         
    ADC10CTL1 = INCH_10 + ADC10DIV_3;                   // Temperature Sensor ADC10CLK/4
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON;  // VR+ = VREF+ and VR- = AVSS, 64 x ADC10CLKs, ADC10 Reference on, ADC10 On/Enable, No interrupt
    
    g_u16EnvTemp = GetTemperature();                    // Get environment temperature TA for comparison
    
}
     
/**
 * Function GetTemperature()
 *
 * Polls the MCU temperature using ADC-conversion and converts it to Celsius
 *
 * @Return unsigned short u16TempDegC - MCU temperature in Celsius
 */
unsigned short GetTemperature(void) {
  
    ADC10CTL0 |= ENC + ADC10SC;         // Enable ADC-conversion and start
    while(ADC10CTL1 & BUSY);            // Wait for it, wait... waaaait, wait for it! NOW!
    long lN_ADC = ADC10MEM;             // Store the value in backpocket
    ADC10CTL0 &= ~ENC;                  // Disable ADC-conversion
    
    unsigned short u16TempDegC = (unsigned short)((423L * lN_ADC - 284412L) >> 10);     // Convert ADC reading to Celcius
    
    return u16TempDegC;                 // Return the converted temperature
    
}