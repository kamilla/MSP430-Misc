/**
 * flipflop.h
 * Header for Dynamic Flip-Flop Flash Memory for MSP430G2553 based AI for motorized car
 * Version 1.0.2.1 - 1.0-rc1
 * 
 * Copyright © 2014 Kamilla Productions Uninc.
 * Written by Joonas Greis <joonas.greis@kamillaproductions.com>
 */

#ifndef _FLIPFLOP_H
#define _FLIPFLOP_H

/* Flip-Flop read & write function declarations */

__ramfunc void write_FlipFlop (unsigned short u16InstructionIndex, unsigned short u16Direction, unsigned short u16Duration);
void read_FlipFlop (unsigned short u16InstructionIndex, unsigned short *u16Direction, unsigned short *u16Duration);


#endif