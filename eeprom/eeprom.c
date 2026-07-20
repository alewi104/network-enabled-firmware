/********************************************************
 * eeprom.c
 *
 * Description:
 * Provides low-level EEPROM read/write functionality using
 * buffered writes and interrupt-driven transfers.
 *
 * Author:  Ahlaireah Lewis
 * Course:  SER486
 * Assignment: Project 2
 * Date:    4/14/2026
 * Revision: 1.0
 ********************************************************/

 #include "eeprom.h"

 #define BUFSIZE 64

 #define EEARH (*(volatile unsigned char *) 0x42) /*EEPROM address register high*/

 #define EEARL (*(volatile unsigned char *) 0x41) /*EEPROM address register low*/

 #define EEDR  (*(volatile unsigned char *) 0x40) /*EEPROM data register*/
 
 #define EECR  (*(volatile unsigned char *) 0x3F) /*EEPROM control register*/
 #define EERIE 3                                  /*EEPROM ready interrupt enable*/
 #define EEMPE 2                                  /*EEPROM master write enable*/
 #define EEPE 1                                   /*EEPROM write enable (after EEMPE is 1)*/
 #define EERE 0                                   /*EEPROM read enable*/


 static unsigned char writebuf[BUFSIZE];
 static unsigned char bufidx;
 static unsigned char writesize;
 static unsigned int writeaddr;
 static volatile unsigned char write_busy;
 
/********************************************************
 * Function: eeprom_writebuf
 *
 * Description:
 * Copies data into an internal buffer and begins asynchronous
 * EEPROM write using interrupts.
 *
 * Parameters:
 * addr - EEPROM starting address
 * buf  - pointer to data buffer
 * size - number of bytes to write
 *
 * Returns:
 * None
 *
 * Hardware Effects:
 * - Enables EEPROM ready interrupt
 * - Starts EEPROM write sequence
 *
 * Notes:
 * - Blocks until no write is in progress
 * - Actual write occurs in ISR
 ********************************************************/
 void eeprom_writebuf(unsigned int addr, unsigned char * buf, unsigned char size){

    /*while write_busy true, do nothing*/
    while(write_busy){}

    /*writeaddr = addr*/
    writeaddr = addr;

    /*write_busy = 1*/
    write_busy = 1;

    /*bufidx = 0*/
    bufidx = 0;

    /*copy buf[0: size-1] to writebuf[0: size-1]*/
    for (int i = 0; i < size; i++){
        writebuf[i] = buf[i];
    }

    /*writesize = size*/
    writesize = size;

    /*enable EEPROM ready interrupts*/
    EECR |= (1 << EERIE);
 }

/********************************************************
 * Function: eeprom_readbuf
 *
 * Description:
 * Reads a block of data from EEPROM into a buffer.
 *
 * Parameters:
 * addr - EEPROM starting address
 * buf  - destination buffer
 * size - number of bytes to read
 *
 * Returns:
 * None
 *
 * Hardware Effects:
 * - Performs direct EEPROM reads
 *
 * Notes:
 * - Blocks if a write is in progress
 ********************************************************/
 void eeprom_readbuf(unsigned int addr, unsigned char * buf, unsigned char size){

    /*while write_busy true, do nothing*/
    while(write_busy){}

    /*read contiguous characters from EEPROM starting at addr
    and place them in buf[0:size-1]*/
    for (int i = 0; i < size; i++){
        EEARH = ((addr >> 8) & 0x03);  //isolate 2 most significant bits
        EEARL = (addr & 0xFF);         //isolate 8 lowest significant bits
        
        EECR |= (1 << EERE);    //trigger read
        buf[i] = EEDR;

        addr++;
    }

 }

/********************************************************
 * Function: eeprom_isbusy
 *
 * Description:
 * Indicates whether an EEPROM write operation is in progress.
 *
 * Parameters:
 * None
 *
 * Returns:
 * int
 *  - 1: busy
 *  - 0: not busy
 *
 * Hardware Effects:
 * None
 ********************************************************/
 int eeprom_isbusy(){

    /*return write_busy == 1*/
    if (write_busy == 0){
        return 0;
    } 
    return 1;
 }

/********************************************************
 * Function: eemprom_unlock
 *
 * Description:
 * Executes the required unlock sequence to allow EEPROM write.
 *
 * Parameters:
 * None
 *
 * Returns:
 * None
 *
 * Hardware Effects:
 * - Sets EEMPE and EEPE bits in EECR register
 *
 * Notes:
 * - Must be executed within strict timing (4 clock cycles)
 ********************************************************/ 
#pragma GCC push_options
 #pragma GCC optimize("Os")
 /*eeprom_unlock*/
 void eemprom_unlock(){

     //start of 4 clock cycle write//
        EECR |= (1 << EEMPE);
        EECR |= (1 << EEPE);
        //end of 4 clock cycle write//

 }
 #pragma GCC pop_options

/********************************************************
 * Function: EEPROM Ready Interrupt (ISR)
 *
 * Description:
 * Handles byte-by-byte EEPROM write from buffer.
 * Disables interrupt when all bytes are written.
 *
 * Parameters:
 * None (ISR)
 *
 * Returns:
 * None
 *
 * Hardware Effects:
 * - Writes data to EEPROM
 * - Updates EEPROM address registers
 * - Enables/disables interrupts
 *
 * Notes:
 * - Automatically triggered by EEPROM hardware
 ********************************************************/
 void __vector_22() __attribute__ ((signal, used, externally_visible));
 void __vector_22(){

    if(bufidx < writesize){

        /*write writebuf[bufidx] to writeaddr in the EEPROM*/
        EEARH = ((writeaddr >> 8) & 0x03);  //isolate 2 most significant bits
        EEARL = (writeaddr & 0xFF);         //isolate 8 lowest significant bits 

        EEDR = writebuf[bufidx];

       eemprom_unlock();
        
        /*writeaddr++*/
        writeaddr++;

        /*bufidx++*/
        bufidx++;

    } else if (bufidx >= writesize){
        
        /*disable EEPROM ready interrupts*/
        EECR &= ~(1 << EERIE);

        /*write_busy = 0*/
        write_busy = 0;
    }
 }

