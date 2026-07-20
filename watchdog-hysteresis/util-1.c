/********************************************************
 * util.c
 *
 * Description:
 * Provides utility functions for updating configuration
 * threshold values with proper validation and persistence.
 *
 * Author: Ahlaireah Lewis
 * Date:     4/23/2026
 * Revision: 1.0
 *
 ********************************************************/

 #include "util.h"
 #include "process_packet.h"
 #include "config.h"

/********************************************************
 * Function: update_tcrit_hi
 * Description:
 * Updates high critical threshold after validation.
 *
 * Arguments:
 *  value - new high critical temperature
 *
 * Returns:
 *  0 = success
 *  1 = invalid value
 *
 * Hardware: writes to non-volatile config
 * Notes: must be > hi_warn and <= 10-bit max (0x3FF)
 ********************************************************/
 int update_tcrit_hi(int value){
    if (value > config.hi_warn && value <= 0x3FF) {

        config.hi_alarm = value;
        config_update();
        return 0;
    }

    return 1;
 }

 /********************************************************
 * Function: update_twarn_hi
 * Description: Updates high warning threshold.
 * Arguments: value - new threshold
 * Returns: 0 (success), 1 (error)
 * Hardware: config write
 * Notes: must remain between lo_warn and hi_alarm
 ********************************************************/
 int update_twarn_hi(int value){
    if (value > config.lo_warn && value < config.hi_alarm){

        config.hi_warn = value;
        config_update();
        return 0;
    }

    return 1;
 }

 /********************************************************
 * Function: update_tcrit_lo
 * Description: Updates low critical threshold.
 * Arguments: value - new threshold
 * Returns: 0 (success), 1 (error)
 * Hardware: config write
 * Notes: must be below lo_warn
 ********************************************************/
 int update_tcrit_lo(int value){
    if (value < config.lo_warn){

        config.lo_alarm = value;
        config_update();
        return 0;
    }

    return 1;
 }

 /********************************************************
 * Function: update_twarn_lo
 * Description: Updates low warning threshold
 * Arguments: value - new threshold
 * Returns: 0 (success), 1 (error)
 * Hardware: config write
 * Notes: must be between lo_alarm and hi_warn
 ********************************************************/
 int update_twarn_lo(int value){
    if (value > config.lo_alarm && value < config.hi_warn){

        config.lo_warn = value;
        config_update();
        return 0;
    }

    return 1;
 }
