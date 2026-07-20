/********************************************************
 * vpd.h
 *
 * Description:
 * Defines VPD structure and interface for accessing
 * device identification data stored in EEPROM.
 *
 * Author:  Ahlaireah Lewis
 * Date:    4/14/2026
 * Revision: 1.0
 ********************************************************/

 #ifndef VPD_H_INCLUDED
 #define VPD_H_INCLUDED

  typedef struct {

    char token[4];
    char model[12];
    char manufacturer[12];
    char serial_number[12];
    unsigned long manufacture_date;
    unsigned char mac_address[6];
    char country_of_origin[4];
    unsigned char checksum;

 } vpd_struct;
 
 /*the vital product data read from the EEPROM at address 0x000.
 This data is repreented as a structure and individual members 
 can be accessed using dot notation*/
 extern vpd_struct vpd;

 /*initializes vpd member data from the EEPROM. If vpd data is invalid 
 after initialization, the EEPROM is written to "factory defaults", and
 the vpd data is reinitialized from the new EEPROM values*/
 void vpd_init();

 #endif //VPD_H_INCLUDED
