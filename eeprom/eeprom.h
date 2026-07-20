/********************************************************
 * eeprom.h
 *
 * Description:
 * Declares EEPROM read/write interface functions.
 *
 * Author:  Ahlaireah Lewis
 * Course:  SER486
 * Assignment: Project 2
 * Date:    4/14/2026
 * Revision: 1.0
 ********************************************************/

 /*places the data (specified by buf and size) into the write buffer
 for later writing to the EEPROM. The addr parameter specifies the location 
 to write the data to. This function should not be called when another write is in progress*/
 void eeprom_writebuf(unsigned int addr, unsigned char * buf, unsigned char size);

 /*reads a specific amount of data (size) from the EEPROM starting at a 
 specified address (addr) and places it in the specified buffer (buf)*/
 void eeprom_readbuf(unsigned int addr, unsigned char * buf, unsigned char size);

 /*returns 0 if write_busy is 0, otherwise, returns 1*/
 int eeprom_isbusy();

