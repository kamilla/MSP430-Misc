/**
 * flipflop.c
 * Dynamic Flip-Flop Flash Memory for MSP430G2553 based AI for motorized car
 * Version 1.0.2.1 - 1.0-rc1
 * 
 * Flash Addresses Flip [0xD400 - 0xDBFF] 4 Segments, 2048 Bytes
 *                 Flop [0xDC00 - 0xE3FF] 4 Segments, 2048 Bytes
 *    
 *      //     Flip_Segment        Flop_Segment
 *      // [ 0xD400 - 0xD5FF ] [ 0xDC00 - 0xDDFF ]
 *      // [ 0xD600 - 0xD7FF ] [ 0xDE00 - 0xDFFF ]
 *      // [ 0xD800 - 0xD9FF ] [ 0xE000 - 0xE1FF ]
 *      // [ 0xDA00 - 0xDBFF ] [ 0xE200 - 0xE3FF ]
 * 
 * Memory available: 4 Segments, 2048 Bytes -> 512 key-value pairs
 * 
 * Fast block-write with 16-bit-write-mode
 * 
 * Param:
 * unsigned short u16InstructionIndex   [1 - 512]       Segment size 512-bytes
 * unsigned short u16Duration           [0 - 65,535]    16-bit, 2 bytes [0x0000 - 0xFFFF]
 * unsigned short u16Direction          [0 - 2]         16-bit, 2 bytes [0x0000 - 0x0002]
 * 
 * (i.e 512 key-value pairs -> if car turns over 512 times in one lap we have memory problem)
 * (i.e if car goes over 65.5 seconds straight we have a memory problem)
 * 
 * Copyright © 2014 Kamilla Productions Uninc.
 * Written by Joonas Greis <joonas.greis@kamillaproductions.com>
 */

/* Includes */     

#include <msp430.h>
#include "flipflop.h"
#include <stdbool.h>

     
/* Boolean to hold information is data written from Flip to Flop or Flop to Flip */
     
__data16 bool bFlipFlop = false;


/* Flip-Flop Address Pointer
 * By defining this global we save 4 Bytes from STACK, but generate 14 Bytes CODE and 2 Bytes mode, so overall we save 2 Bytes RAM
 * Initialized variables will be placed in the segment DATA16_I in RAM, and initializer data in segment DATA16_ID in ROM.*/   

__data16 unsigned short *prgFlashSegment[2] = {(unsigned short *) 0xD400, (unsigned short *) 0xDC00};


/* Flip-Flop Flash writer function. __ramfunc function keyword tells the compiler to place function in RAM */

__ramfunc void write_FlipFlop (unsigned short u16InstructionIndex, unsigned short u16Direction, unsigned short u16Duration) {   
    
    prgFlashSegment[0] = (unsigned short *) 0xD400;                     // Reset the Address Pointer for Flip-Segment
    prgFlashSegment[1] = (unsigned short *) 0xDC00;                     // Reset the Address Pointer for Flop-Segment
    
    unsigned short u16DataSize = (u16InstructionIndex - 1) * 4;         // Old data size to be copied
    unsigned short cFlashBlock[32];                                     // Array to hold block-data to be copied
    unsigned short u16BlockSize = 64;                                   // Initial block size
    
    unsigned short u16Blocks = u16DataSize >> 6;                        // Count how many iterations we need, iterations * blocksize
    unsigned short u16BlockLast = u16DataSize - (u16Blocks << 6);       // Count the size of last block. We don't want modulus %, because it's freaking slow on MSP430
    u16Blocks++;                                                        

    for (int n = 0; n < u16Blocks; n++) {                               // Iterate all the blocks that needs to be copied  
        
        if ((n + 1) == u16Blocks) u16BlockSize = u16BlockLast;          // If on the last block, set block size to left data size
        
        while(FCTL3&BUSY);                                              // Check BUSY flag; Important if running this code from RAM
        FCTL1 = FWKEY + ERASE;                                          // Set Erase bit
        FCTL3 = FWKEY;                                                  // Clear Lock bit

        if (n - (n >> 3 << 3) == 0) *prgFlashSegment[!bFlipFlop] = 0;   // Erase segment if needed. Saves 8 bytes CODE and 8 bytes DATA memory compared to if (n==0||n==8||n==16||n==24)
      
        while(FCTL3&BUSY);                                              // Check the BUSY flag
        FCTL1 = FWKEY + BLKWRT +  WRT;                                  // Set BLKWRT and WRT bits for block write operation
        
        
        // Keep the scope of cFlashBlock[] short as possible, we save 14 Bytes CODE, 14 Bytes DATA and 2 Bytes STACK usage with placing this here instead of start of the loop
        
        for (unsigned short u16FlashAddress = 0; u16FlashAddress < (u16BlockSize >> 1); u16FlashAddress++) {    // Iterate trough block by 16-bit parts (every second byte)
            
            cFlashBlock[u16FlashAddress] = *prgFlashSegment[bFlipFlop]++;       // Copy old data from Flop/Flip Flash-segment to block array
            
        }
        
        
        for (unsigned short u16FlashAddress = 0; u16FlashAddress < (u16BlockSize >> 1); u16FlashAddress++) {    // Iterate trough block by 16-bit parts (every second byte)

            *prgFlashSegment[!bFlipFlop]++ = cFlashBlock[u16FlashAddress];      // Copy old data from block array to Flop/Flip Flash-segment 
            while(!(FCTL3 & WAIT));                                             // Wait for Flash to be ready
            
        }
        
        
        if ((n + 1) == u16Blocks) {                                     // If on the last block write new data
            
            *prgFlashSegment[!bFlipFlop]++ = u16Direction;              // Write direction to Flop/Flip Flash-segment
            while(!(FCTL3 & WAIT));                                     // Wait for Flash to be ready
            *prgFlashSegment[!bFlipFlop] = u16Duration;                 // Write duration Flop/Flip Flash-segment
            while(!(FCTL3 & WAIT));                                     // Wait for Flash to be ready
            
        }

        FCTL1 = FWKEY;                                                  // Clear WRT bit
  
    }
  
    while(FCTL3&BUSY);                                                  // Check BUSY flag
    FCTL3 = FWKEY + LOCK;                                               // Set LOCK bit
    
    bFlipFlop ^= true;                                                  // Flip the Flop

}


/* Flip-Flop Flash reader function. */

void read_FlipFlop (unsigned short u16InstructionIndex, unsigned short *u16Direction, unsigned short *u16Duration) {
    
    unsigned short u16FlipFlopSegmentStartAddress;                      // Initialize to starting const to segment that has the data
    if(bFlipFlop) u16FlipFlopSegmentStartAddress = 0xD400;              // Flip-segment
    else u16FlipFlopSegmentStartAddress = 0xDC00;                       // Flop-segment
    
    unsigned short *pAddressPointer = (unsigned short *) ((u16InstructionIndex - 1) * 4 + u16FlipFlopSegmentStartAddress);      // Initialize the address pointer to given index
    *u16Direction = *pAddressPointer++;                                 // Read direction data
    *u16Duration = *pAddressPointer;                                    // Read duration data

}