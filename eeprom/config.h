/******************************************************** 
 * config.h
 *
 * Description:
 * Defines configuration data structure and public interface
 * for managing persistent configuration stored in EEPROM.
 *
 * Author:  Ahlaireah Lewis
 * Course:  SER486
 * Assignment: Project 2
 * Date:    4/14/2026
 * Revision: 1.0
 ********************************************************/

 #ifndef CONFIG_H_INCLUDED
 #define CONFIG_H_INCLUDED
 typedef struct {
    char token[4];
    unsigned int hi_alarm;
    unsigned int hi_warn;
    unsigned int lo_alarm;
    unsigned int lo_warn;
    char use_static_ip;
    unsigned char static_ip[4];
    unsigned char checksum; 
 } config_struct;
 
 /*the configuration data read from the EEPROM at address 0x040. This data is 
 represented as a structure and individual members can be accessed using dot 
 notation. The data also acts as a write cache. Data may be read/written while 
 EEPROM updates are in progress*/
 extern config_struct config;

 /*initializes config member data from the EEPROM. If config data is invalid
 after initialization, the EEPROM is written to "factory defaults", and the config data
 is reinitialized from the new EEPROM values*/
 void config_init();


 /*if config has been modified and eeprom is not busy, write all of the 
 configuration data to the eeprom write buffer*/
 void config_update();


 /*set the modified flag. This function should be called any time the 
 config data is modified */
 void config_set_modified();


#endif //CONFIG_H_INCLUDED
