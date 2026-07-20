/********************************************************
 * vpd.c
 *
 * Description:
 * Manages Vital Product Data (VPD) stored in EEPROM.
 * Includes validation and fallback to factory defaults.
 *
 * Author:  Ahlaireah Lewis
 * Date:    4/14/2026
 * Revision: 1.0
 ********************************************************/

 #include "vpd.h"
 #include "eeprom.h"
 #include "util.h"


 /*the vital product data read from the EEPROM at address 0x000.
 This data is repreented as a structure and individual members 
 can be accessed using dot notation*/
 vpd_struct vpd = {};

 /*the factory defaults for the vpd*/
 static vpd_struct defaults = { 
    "SER",
    "Ahlaireah",
    "Lewis",
    "A1B2C3D4E5F",
    0,
    {0x40, 0x48, 0x4C, 0x4C, 0x45, 0x57}, 
    "USA",
    0
 };

/********************************************************
 * Function: vpd_is_data_valid (static)
 *
 * Description:
 * Validates VPD using token "SER" and checksum.
 *
 * Parameters:
 * None
 *
 * Returns:
 * int
 *  - 1: valid
 *  - 0: invalid
 *
 * Hardware Effects:
 * None
 ********************************************************/
 static int vpd_is_data_valid(){

    static unsigned char* p = (unsigned char*)&vpd;

    /*result = 1*/
    static int result = 1;

    /*vpd.token is not "SER"*/
    if (vpd.token[0] != 'S' || vpd.token[1] != 'E' || vpd.token[2] != 'R'){
        result = 0;
    } 
    
    /*checksum not valid*/
    else if (!is_checksum_valid(p, sizeof(vpd_struct))){
        result = 0;
    } 

    /*return result*/
    return result;
 }

 /********************************************************
 * Function: vpd_write_defaults (static)
 *
 * Description:
 * Writes default VPD values to EEPROM with updated checksum.
 *
 * Parameters:
 * None
 *
 * Returns:
 * None
 *
 * Hardware Effects:
 * - Writes to EEPROM
 ********************************************************/
 static void vpd_write_defaults(){

    static unsigned char* p = (unsigned char*)&defaults;

    /*update_checksum() for defaults*/
    update_checksum(p, sizeof(vpd_struct));

    /*write defaults to eeprom*/
    eeprom_writebuf(0x000, p, sizeof(vpd_struct));

 }

/********************************************************
 * Function: vpd_init
 *
 * Description:
 * Loads VPD from EEPROM. If invalid, restores defaults.
 *
 * Parameters:
 * None
 *
 * Returns:
 * None
 *
 * Hardware Effects:
 * - Reads from EEPROM
 * - May write defaults
 *
 * Notes:
 * - Blocks until EEPROM is ready
 ********************************************************//
 void vpd_init(){
    
    /*if eeprom_isbusy is true, loop*/
    while(eeprom_isbusy()){};

    /*initialize the vpd data w/ eeprom.readbuf()*/
    static unsigned char* p = (unsigned char*)&vpd;
    eeprom_readbuf(0x000, p, sizeof(vpd_struct));

    /*if data is invalid*/
    if(!vpd_is_data_valid()){

        /*write_defaults()*/
        vpd_write_defaults();

        /*initialize vpd data w/ eeprom_readbuf()*/
        eeprom_readbuf(0x000, p, sizeof(vpd_struct));
    } else {
        return;
    }
 }
