/**********************************************************
* led.c
* Authored by: Ahlaireah Lewis
* Implements an LED blink finite state machine (FSM).
*
* - Interprets a message string ('.', '-', ' ')
* - Uses delay instance 0 for timing control
* - Non-blocking design driven by led_update()
*
* FSM States:
*   Phase 1 - Output symbol (dot/dash/space)
*   Phase 2 - Inter-symbol gap (100 ms OFF)
*
* Internal State:
*   blink_msg   - pointer to message string
*   blink_pos   - current character index
*   blink_state - FSM state (1 or 2)
*
* Notes:
* - Timing handled via delay module
**********************************************************/

#include "delay.h"
#include "led.h"
#include "uart.h"

static char * blink_msg;
static unsigned int blink_pos;
static unsigned char blink_state;

/*--------------------------------------------------------
* led_set_blink
*
* Initializes a new blink pattern.
*
* - Sets message string
* - Resets position and FSM state
* - Clears delay and turns LED off
*
* Parameters:
*   msg - pointer to null-terminated pattern string
--------------------------------------------------------*/
void led_set_blink(char* msg){

    /*initialize message*/
    blink_msg = msg;
    blink_pos = 0;

    /*reset FSM*/
    blink_state = 1;
    delay_set(0, 0);
    led_off();
}

/*--------------------------------------------------------
* led_update
*
* Advances the LED finite state machine.
*
* Behavior:
* - Evaluates current state
* - Sets LED and delay on state entry
* - Transitions when delay completes
*
* Notes:
* - Non-blocking; must be called continuously
* - Relies on delay_isdone() for timing
--------------------------------------------------------*/
void led_update()
{
    if (blink_msg == '\0'){
        return;
    } else if (!delay_isdone(0)){
        return;
    } else {

    switch (blink_state)
    {
        //phase1
    case 1:

        //on entry
        if (blink_msg[blink_pos] == ' '){
             delay_set(0, 1000); led_off();
        } else if (blink_msg[blink_pos] == '-'){
            delay_set(0, 750); led_on();
        } else if (blink_msg[blink_pos] == '.'){
            delay_set(0, 250); led_on();
        }else {
            delay_set(0, 0);led_off();
        } 
        
        if (delay_isdone(0) && (blink_msg[blink_pos] == ' ')){
            if (blink_msg[blink_pos + 1] != '\0')
            {
                blink_pos++;
            } else {
                blink_pos = 0;
            }
            blink_state = 1;
        } else if (delay_isdone(0) && (blink_msg[blink_pos] != ' ')){
            blink_state = 2;
        }
        
        break;

        //Phase 2
    case 2:
        delay_set(0, 100);
        led_off();

        if (delay_isdone(0))
        {
            if (blink_msg[blink_pos + 1] != '\0')
            {
                blink_pos++;
            }
            else
            {
                blink_pos = 0;
            }
        }
        blink_state = 1;
        break; 

    }
}
}
