/**********************************************************
* main.c
*
*
*/

#include "delay.h"
#include "led.h"
#include "timer1.h"
#include "rtc.h"
#include "uart.h"


int main() {

    /*initialize state*/
    uart_init();
    led_init();
    rtc_init();

    led_set_blink("--- -.-");
    rtc_set_by_datestr("01/01/2019 00:00:00");

    uart_writestr("SER 486 Project 1 - Ahlaireah Lewis\n\r");

    /*get baseline performance*/
    signed long c1 = 0;
    delay_set(1, 10000);
    while(!delay_isdone(1)){
        c1++;
    }

    /*measure performance with led_update*/
    signed long c2 = 0;
    delay_set(1, 10000);
    while(!delay_isdone(1)){
        led_update();
        c2++;
    }

    /*display the results*/
    uart_writedec32(c1); 
    uart_writestr(" ");
    uart_writedec32(c2); 
    uart_writestr("\n\r");


    while (1) {

        delay_set(1, 500);
        while(!delay_isdone(1)){
            
            uart_writestr(rtc_get_date_string());
            uart_writestr("\r");
        }

    }
    return 0;
}