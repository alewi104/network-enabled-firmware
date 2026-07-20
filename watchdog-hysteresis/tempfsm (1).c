/********************************************************
 * tempfsm.c
 *
 * Description:
 * Implements the temperature finite state machine (FSM)
 * for monitoring sensor readings and triggering alarms,
 * logs, and LED indicators based on threshold crossings.
 *
 * Author: Ahlaireah Lewis
 * Date:     4/23/2026
 * Revision: 1.0
 *
 ********************************************************/

 #include "alarm.h"
 #include "led.h"
 #include "log.h"
 #include "tempfsm.h"

/* FSM state definitions */
 #define NORM1 1
 #define NORM2 2
 #define NORM3 3
 #define WARN_LO1 4
 #define WARN_LO2 5
 #define CRITICAL_LO 6
 #define WARN_HI1 7
 #define WARN_HI2 8
 #define CRITICAL_HI 9

/* holds current FSM state */
 static unsigned char state;


 /********************************************************
 * Function: tempfsm_init
 * Description: Initializes the temperature FSM and LED.
 * Arguments: none
 * Returns: none
 * Hardware: LED state modified
 * Notes: Resets FSM to default state and turns LED off
 ********************************************************/
 void tempfsm_init(){

    tempfsm_reset();
    led_off();
 }

 /********************************************************
 * Function: tempfsm_reset
 * Description: Resets FSM to initial normal state.
 * Arguments: none
 * Returns: none
 * Hardware: none
 * Notes: Default state is NORM1
 ********************************************************/
 void tempfsm_reset(){

    state = NORM1;
 }
/********************************************************
 * Function: tempfsm_update
 * Description:
 * Updates FSM state based on current temperature and
 * configured threshold values. Triggers alarms, logs,
 * and LED behavior on transitions.
 *
 * Arguments:
 *  current - current temperature reading
 *  hicrit  - high critical threshold
 *  hiwarn  - high warning threshold
 *  locrit  - low critical threshold
 *  lowarn  - low warning threshold
 *
 * Returns: none
 * Hardware:
 *  - LED updated
 *  - alarm module triggered
 *  - log module updated
 *
 * Notes:
 *  - FSM uses hysteresis (multiple states per region)
 *  - LED patterns:
 *      "-" = warning
 *      "." = critical
 *      " " = normal
 ********************************************************/
 void tempfsm_update(int current, int hicrit, int hiwarn, int locrit, int lowarn){

    switch (state){

        case NORM1:

            if (current <= lowarn)
            {
                alarm_send(EVENT_LO_WARN);
                log_add_record(EVENT_LO_WARN);

                led_set_blink("-");

                state = WARN_LO1;
                break;
            }

            if (current >= hiwarn)
            {
                alarm_send(EVENT_HI_WARN);
                log_add_record(EVENT_HI_WARN);

                led_set_blink("-");

                state = WARN_HI1;
                break;
            }
            break;

        case NORM2:

            if (current <= lowarn)
            {

                led_set_blink("-");

                state = WARN_LO1;
                break;
            }

            if (current >= hiwarn)
            {
                alarm_send(EVENT_HI_WARN);
                log_add_record(EVENT_HI_WARN);

                led_set_blink("-");

                state = WARN_HI1;
                break;
            }
            break;

        case NORM3:

            if (current <= lowarn)
            {
                alarm_send(EVENT_LO_WARN);
                log_add_record(EVENT_LO_WARN);

                led_set_blink("-");

                state = WARN_LO1;
                break;
            }

            if (current >= hiwarn)
            {

                led_set_blink("-");

                state = WARN_HI1;
                break;
            }
            break;

        case WARN_LO1:

            if (current <= locrit)
            {
                alarm_send(EVENT_LO_ALARM);
                log_add_record(EVENT_LO_ALARM);

                led_set_blink(".");

                state = CRITICAL_LO;
                break;
            }

            if (current > lowarn)
            {

                led_set_blink(" ");

                state = NORM2;
                break;
            }
            break;

        case WARN_LO2:

            if (current <= locrit)
            {

                led_set_blink(".");

                state = CRITICAL_LO;
                break;
            }

            if (current > lowarn)
            {

                led_set_blink(" ");

                state = NORM2;
                break;
            }
            break;

        case WARN_HI1:

            if (current < hiwarn)
            {

                led_set_blink(" ");

                state = NORM3;
                break;
            }

            if (current >= hicrit)
            {
                alarm_send(EVENT_HI_ALARM);
                log_add_record(EVENT_HI_ALARM);

                led_set_blink(".");

                state = CRITICAL_HI;
                break;
            }
            break;

        case WARN_HI2:

            if (current < hiwarn)
            {

                led_set_blink(" ");

                state = NORM3;
                break;
            }

            if (current >= hicrit)
            {

                led_set_blink(".");

                state = CRITICAL_HI;
                break;
            }
            break;

        case CRITICAL_LO:

            if (current > locrit)
            {

                led_set_blink("-");
            }
            state = WARN_LO2;
            break;

        case CRITICAL_HI:

            if (current < hicrit)
            {

                led_set_blink("-");
            }
            state = WARN_HI2;
            break;
    }
 }

