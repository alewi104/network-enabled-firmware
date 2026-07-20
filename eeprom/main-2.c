/**********************************************************
 * main.c
 *
 * Description:
 * Entry point for the embedded system. Initializes all
 * subsystems and runs the main control loop.
 *
 * Author:  Ahlaireah Lewis
 * Course:  SER486
 * Assignment: Project 2
 * Date:    4/14/2026
 **********************************************************/

#include "led.h"
#include "uart.h"
#include "config.h"
#include "log.h"
#include "rtc.h"
#include "vpd.h"
#include "eeprom.h"
#include "util.h"

/********************************************************
 * Function: main
 *
 * Description:
 * Initializes hardware modules and executes the main loop.
 * Handles periodic updates for LED, logging, and config.
 *
 * Parameters:
 * None
 *
 * Returns:
 * int (never reached)
 *
 * Hardware Effects:
 * - Initializes UART, EEPROM, LED, RTC, logging
 * - Continuously updates system components
 *
 * Notes:
 * - Infinite loop system
 * - EEPROM dump occurs once after initialization
 ********************************************************/
int main() {
    
    /*initialize hardware*/
    uart_init();
    config_init();
    led_init();
    log_init();
    rtc_init();
    vpd_init();

    /*set led blink and rtc date*/
    led_set_blink("--- -.-");
    rtc_set_by_datestr("01/01/2019 00:00:00");

    /*write to console*/
    uart_writestr("SER 486 Project 2 - Ahlaireah Lewis \n\r");
    uart_writestr("Ahlaireah\n\r");
    uart_writestr("Lewis\n\r");
    uart_writestr("ASU\n\r");

    config.use_static_ip = 1;
    config_set_modified();

    log_clear();
    log_add_record(0xaa);
    log_add_record(0xbb);
    log_add_record(0xcc);

    int dumped = 0;

    /*start of loop*/
    while (1) {

        led_update();
        log_update();
        config_update();

        if ((!eeprom_isbusy()) && (!dumped)){
            dump_eeprom(0, 0x100);
            dumped = 1;
        }

    }
    return 0;
}
