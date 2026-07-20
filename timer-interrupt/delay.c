/**********************************************************
* delay.c
*
* SER 486 Project 1
* 4/12/26 [Spring 2026]
* Author: Ahlaireah Lewis
* 
* Implements a non-blocking delay system using Timer0.
*
* - Timer0 runs in CTC mode to generate a 1 ms tick.
* - An interrupt increments delay counters asynchronously.
* - Multiple delay instances are supported (2 per spec).
*
* Internal State:
*   count[]  - elapsed time (ms) for each delay instance
*   limit[]  - target delay (ms) for each instance
*
* Notes:
* - count[] is updated inside the ISR → must be volatile
* - All access to shared state is protected via CLI/SREG
**********************************************************/

#define TCCR0A (*(volatile unsigned char *) 0x44) //Timer/Counter0 Control Register A
#define WGM01 1
#define WGM00 0

#define TCCR0B (*(volatile unsigned char *) 0x45) //Timer/Counter0 Control Register B
#define WGM02 3
#define CS02 2
#define CS01 1
#define CS00 0

#define TIMSK0 (*(volatile unsigned char *) 0x6E) //Timer/Counter0 Interrupt Mask Register
#define OCIEA 1

#define OCR0A (*(volatile unsigned char *) 0x47) //Output Compare Register A for Timer/Counter0

#define SREG (*(volatile unsigned char *) 0x5F) //Status Register

static volatile unsigned int count[];
static unsigned int limit[];
static unsigned char initialized;

/*--------------------------------------------------------
* init
*
* Configures Timer0 to generate a 1 ms interrupt tick.
*
* - Sets CTC mode 
* - Sets prescaler to 64
* - Enables Output Compare A interrupt
*
* Called lazily on first delay_set() invocation.
--------------------------------------------------------*/
static void init() {

    /*set timer0 compare for 1ms (1000Hz) tick*/
    OCR0A = 0xF9; //for prescaler of 256

    /*set CTC mode 010*/
    TCCR0B &= ~(1 << WGM02);
    TCCR0A |= (1 << WGM01);
    TCCR0A &= ~(1 << WGM00);

    /*set correct clock divisor 011*/
    TCCR0B &= ~(1 << CS02);
    TCCR0B |= (1 << CS01);
    TCCR0B |= (1 << CS00);
    
    /*enable interrupts on output compare A*/
    TIMSK0 |= (1 << OCIEA);

    /*Stop further initialization by setting initialized to 1*/
    initialized = 1;
}

/*--------------------------------------------------------
* delay_get
*
* Returns the elapsed time (ms) for a given delay instance.
*
* Parameters:
*   num - delay instance index (0 or 1)
*
* Returns:
*   Current count value in milliseconds
*
* Notes:
* - Access is protected to prevent ISR race conditions
--------------------------------------------------------*/
unsigned int delay_get(unsigned int num){
    /*get global interrupt enable bit state*/
    unsigned char state = SREG;

    /*disable interrupts*/
    __builtin_avr_cli();

    /*get the count[n] value*/
    unsigned int value = count[num];

    /*restore global interrupt state*/
    SREG |= state;

    /*return the count value*/
    return value;
}

/*--------------------------------------------------------
* delay_set
*
* Initializes a delay instance.
*
* - Sets the target delay (limit)
* - Resets the elapsed time (count) to zero
*
* Parameters:
*   num  - delay instance index (0 or 1)
*   time - delay duration in milliseconds
*
* Notes:
* - Automatically initializes Timer0 on first use
* - Access is interrupt-safe
--------------------------------------------------------*/
void delay_set(unsigned int num, unsigned int time){

    if (initialized == 0) {
        /*initialize the delay counter*/
        init();
    }

    /*get global interrupt enable bit state*/
    unsigned char state = SREG;

    /*disable interrrupts*/
    __builtin_avr_cli();

    /*set the limit for delay[n] and clear the count for delay[n]*/
    limit[num] = time;
    count[num] = 0;

    /*restore global interrupt state*/
    SREG |= state;
}

/*--------------------------------------------------------
* delay_isdone
*
* Checks whether a delay has completed.
*
* Parameters:
*   num - delay instance index
*
* Returns:
*   1 if count >= limit
*   0 otherwise
*
* Notes:
* - Non-blocking; intended for FSM polling
--------------------------------------------------------*/
unsigned int delay_isdone(unsigned int num){

    /*result = 0*/
    unsigned int result = 0;

    if(count[num] == limit[num]){
        /*result = 1*/
        result = 1;
    }

    /*return result*/
    return result;

}

/*--------------------------------------------------------
* __vector_14 (Timer Compare Match ISR)
*
* Invoked every 1 ms by Timer0.
*
* - Increments each delay counter
* - Ensures count does not exceed limit
*
* Notes:
* - Executes asynchronously to main program flow
* - Must remain lightweight (no blocking operations)
--------------------------------------------------------*/
void __vector_14(void) __attribute__ ((signal, used, externally_visible));
void __vector_14(void){
    /*for each instance of delay, increment its count as long as it
      is not equal to or greater than its limit 
    */

   for (int i = 0; i < limit[i]; i++){
    count[i]++;
   }
}
