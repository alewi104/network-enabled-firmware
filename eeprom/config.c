/******************************************************** 
 * config.c
 *
 * Description:
 * Implements configuration management for device settings.
 * Configuration data is cached in RAM and persisted to EEPROM.
 * Includes validation using token and checksum, and supports
 * restoring factory defaults if corruption is detected.
 *
 * Author:  Ahlaireah Lewis
 * Date:    4/14/2026
 * Revision: 1.0
 ********************************************************/
#include "config.h"
#include "eeprom.h"
#include "util.h"

 config_struct config = {};

 static config_struct defaults = {

    "ASU",
    0x3FF,
    0x3FE,
    0x0000,
    0x0001,
    0,
    {192, 168, 1, 100},
    0
 };

 static unsigned char modified;


/********************************************************
 * Function: config_update
 *
 * Description:
 * Writes the current configuration structure to EEPROM if
 * the data has been modified and EEPROM is not busy.
 * Also recalculates and updates the checksum before writing.
 *
 * Parameters:
 * None
 *
 * Returns:
 * None
 *
 * Hardware Effects:
 * - Initiates EEPROM write sequence via eeprom_writebuf()
 *
 * Notes:
 * - Must be called periodically (e.g., main loop)
 * - Does nothing if EEPROM is busy or config not modified
 ********************************************************/
void config_update(){

    /*if eeprom is not busy and modified is true */
    if (!eeprom_isbusy() && modified == 1) {
        
        unsigned char* p = (unsigned char*)&config;
        
        /*update the checksum*/
        update_checksum(p, sizeof(config_struct));

        /*write config to eeprom*/
        eeprom_writebuf(0x040, p, sizeof(config_struct));

        /*clear the modified flag*/
        modified = 0;
    }
}


/********************************************************
 * Function: config_set_modified
 *
 * Description:
 * Sets the modified flag indicating configuration data
 * has changed and must be written to EEPROM.
 *
 * Parameters:
 * None
 *
 * Returns:
 * None
 *
 * Hardware Effects:
 * None
 *
 * Notes:
 * - Must be called after ANY change to config struct
 ********************************************************/
void config_set_modified(){
    modified = 1;
}

/********************************************************
 * Function: config_is_data_valid (static)
 *
 * Description:
 * Validates configuration data by checking:
 * 1. Token matches "ASU"
 * 2. Checksum is correct
 *
 * Parameters:
 * None
 *
 * Returns:
 * int
 *  - 1: valid data
 *  - 0: invalid data
 *
 * Hardware Effects:
 * None
 *
 * Notes:
 * - Used internally during initialization
 ********************************************************/
static int config_is_data_valid(){

    static unsigned char* p  = (unsigned char*)&config;

    /*result = 1*/
    static int result = 1;

    if(config.token[0] != 'A' || config.token[1] != 'S' || config.token[2] != 'U' ){
        result = 0;
    } else if (!is_checksum_valid(p, sizeof(config_struct))){
        result = 0;
    }

    return result;
}

/********************************************************
 * Function: config_write_defaults (static)
 *
 * Description:
 * Writes factory default configuration values to EEPROM.
 * Also computes checksum before writing.
 *
 * Parameters:
 * None
 *
 * Returns:
 * None
 *
 * Hardware Effects:
 * - Writes full config struct to EEPROM
 *
 * Notes:
 * - Called only when stored config is invalid
 ********************************************************/
static void config_write_defaults(){

    static unsigned char* p = (unsigned char*)&defaults;
    
    /*update_checksum() for defaults*/
    update_checksum(p, sizeof(config_struct));

    /*write defaults to eeprom*/
    eeprom_writebuf(0x040, p, sizeof(config_struct));
}

/********************************************************
 * Function: config_init
 *
 * Description:
 * Initializes configuration data from EEPROM.
 * If data is invalid, writes factory defaults and reloads.
 *
 * Parameters:
 * None
 *
 * Returns:
 * None
 *
 * Hardware Effects:
 * - Reads from EEPROM
 * - May write defaults to EEPROM
 *
 * Notes:
 * - Blocks until EEPROM is not busy
 * - Must be called before using config
 ********************************************************/
void config_init(){

    /*if eeprom is busy, do nothing and loop*/
    while(eeprom_isbusy()){};

    /*initialize config with eeprom readbuf()*/
    unsigned char* p = (unsigned char*)&config;
    eeprom_readbuf(0x040, p, sizeof(config_struct));

    /*if data is invalid*/
    if (!config_is_data_valid()){

        /*write_defaults()*/
        config_write_defaults();

        /*initialize config w/ eeprom readbuf()*/
        eeprom_readbuf(0x040, p, sizeof(config_struct));

        /*clear the modified flag*/
        modified = 0;
    }
}
