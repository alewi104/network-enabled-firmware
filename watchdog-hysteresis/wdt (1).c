/********************************************************
 * wdt.c
 *
 * Description:
 * Implements watchdog timer (WDT) functionality including
 * initialization, reset handling, and interrupt servicing.
 *
 * Author: Ahlaireah Lewis
 * Course: SER486
 * Assignment: Project 3
 * Date:     4/23/2026
 * Revision: 1.0
 *
 ********************************************************/

 #include "led.h"
 #include "log.h"
 #include "config.h"
 #include "delay.h"
 #include "wdt.h"
/* WDT control register and bit definitions */
 #define WDTCSR (*(volatile unsigned char*) 0x60)  //WDT control status register
 #define WDP3 5  //wdt prescaler
 #define WDP2 2  //wdt prescaler
 #define WDP1 1  //wdt prescaler
 #define WDP0 0  //wdt prescaler
 #define WDIF 7  //watchdog interrupt flag
 #define WDIE 6  //watchdog interrupt enable
 #define WDCE 4  //watchdog change enable. set before WDE
 #define WDE  3  //watchdog system reset enable
 
 //#define MCUSR (*(volatile unsigned char*) 0x54) //MCU status register
 //#define WDRF 3 //Watchdog system reset flag
 
 #pragma GCC push_options
 #pragma GCC optimize("Os")

 /********************************************************
 * Function: wdt_prescaler_change
 * Description:
 * Configures WDT timeout period (~2 seconds).
 * Arguments: none
 * Returns: none
 * Hardware: modifies WDTCSR register
 * Notes: must complete within 4 clock cycles
 ********************************************************/
 void wdt_prescaler_change(){

    /**/
    WDTCSR |= (1 << WDCE) | (1 << WDE);

    /*initializes wdt for timeout period of 2 seconds (0111)*/
    /*start of 4 clock cycle write*/
    WDTCSR = (1 << WDE) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0);
    /*end of 4 clock cycle write*/
 }

 /********************************************************
 * Function: wdt_enable_interrupt_and_sys_reset_mode
 * Description:
 * Enables both interrupt and system reset modes.
 * Arguments: none
 * Returns: none
 * Hardware: WDTCSR register
 * Notes: interrupt fires first, then reset
 ********************************************************/
 void wdt_enable_interrupt_and_sys_reset_mode(){

   /*enables interrupt and then system reset mode*/
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = (1 << WDE) | (1 << WDIE) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0);

 }

 /********************************************************
 * Function: wdt_disable_interrupt
 * Description: Disables WDT interrupt mode.
 * Arguments: none
 * Returns: none
 * Hardware: WDTCSR register
 ********************************************************/
 void wdt_disable_interrupt() {

    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = (1 << WDE) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0);
 }
 #pragma GCC pop_options
 
/********************************************************
 * Function: wdt_init
 * Description:
 * Initializes WDT for interrupt + reset behavior.
 *
 * Arguments: none
 * Returns: none
 *
 * Hardware:
 *  - WDTCSR register
 *
 * Notes:
 *  - Must call wdt_reset() periodically (<2s)
 *  - Handles pending WDT interrupt flag if set
 ********************************************************/
 void wdt_init(){

    if (WDTCSR & (1 << WDIF)) {
      WDTCSR &= ~(1 << WDIF);
      wdt_force_restart();
    }

    wdt_prescaler_change(); 
    wdt_enable_interrupt_and_sys_reset_mode();

 }

 /********************************************************
 * Function: wdt_reset
 * Description: Resets watchdog timer countdown.
 * Arguments: none
 * Returns: none
 * Hardware: WDT reset instruction
 ********************************************************/
 void wdt_reset(){

    __builtin_avr_wdr();
 }

/********************************************************
 * Function: wdt_force_restart
 * Description:
 * Forces system restart via watchdog timeout.
 *
 * Arguments: none
 * Returns: none
 *
 * Hardware:
 *  - disables WDT interrupt
 *  - waits for reset
 *
 * Notes: infinite loop until reset occurs
 ********************************************************/
 void wdt_force_restart(){

    wdt_disable_interrupt();
    while(1){}   // wait for WDT reset

 }

/********************************************************
 * ISR: __vector_6 (WDT interrupt)
 * Description:
 * Handles first-stage watchdog timeout.
 *
 * Actions:
 *  - turns LED on
 *  - logs WDT event
 *  - attempts to flush log and config to memory
 *
 * Notes:
 *  - system reset will follow after ISR completes
 ********************************************************/
 void __vector_6() __attribute__ ((signal, used, externally_visible));
 void __vector_6(){

    /*turns on the led*/
    led_on();

    /*adds event to the system event log*/
    log_add_record(EVENT_WDT);

    for (int i = 0; i < 16; i++){
      log_update_noisr();
    }
    config_update_noisr();

 }

