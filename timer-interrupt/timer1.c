/**********************************************************
* timer1.c
* Auhtored by: Ahlaireah Lewis
* Implements a 1-second time base using Timer1.
*
* - Timer1 runs in CTC mode
* - OCR1A is set to generate a 1 Hz interrupt
* - ISR increments a global seconds counter
*
* Internal State:
*   count - number of elapsed seconds
*
* Notes:
* - count is updated in ISR → must be volatile
* - Access is protected when read from main
**********************************************************/

#define TCCR1A (*(volatile unsigned char *) 0x80) //Timer/Counter1 Control Register A
#define WGM11 1
#define WGM10 0 

#define TCCR1B (*(volatile unsigned char *) 0x81) //Timer/Counter1 Control Register B
#define WGM13 4
#define WGM12 3
#define CS12 2
#define CS11 1
#define CS10 0

#define TCCR1C (*(volatile unsigned char *) 0x82) //Timer/Counter1 Control Register C

#define TIMSK1 (*(volatile unsigned char *) 0x6F) //Timer/Counter1 Interrupt Mask Register
#define OCIEA 1

#define OCR1AL (*(volatile unsigned char *) 0x88) //Output Compare Register A (Low) for Timer/Counter1
#define OCR1AH (*(volatile unsigned char *) 0x89) //Output Compare Register A (High) for Timer/Counter1

#define SREG (*(volatile unsigned char *) 0x5F) //Status Register

static volatile unsigned long count;

/*--------------------------------------------------------
* timer1_init
*
* Configures Timer1 to generate a 1 second interrupt.
*
* - Sets CTC mode (OCR1A as TOP)
* - Loads OCR1A for 1 Hz (based on 16 MHz clock, /1024)
* - Enables Output Compare A interrupt
*
* Notes:
*
--------------------------------------------------------*/
void timer1_init(){

    /*set timer compare for 1s*/
    OCR1AH |= 0x3D;
    OCR1AL |= 0x08;

    /*set CTC mode 1100*/
    TCCR1B |= (1 << WGM13);
    TCCR1B |= (1 << WGM12);
    TCCR1A &= ~(1 << WGM11);
    TCCR1A &= ~(1 << WGM10);

    /*set clock divisor 101*/
    TCCR1B |= (1 << CS12);
    TCCR1B &= ~(1 << CS11);
    TCCR1B |= (1 << CS10);


    /*enable interrupts on output compare A*/
    TIMSK1 |= (1 << OCIEA);
}

/*--------------------------------------------------------
* timer1_get
*
* Returns the number of elapsed seconds.
*
* Returns:
*   Current value of the seconds counter
*
* Notes:
* - Access is protected to avoid ISR race conditions
--------------------------------------------------------*/
unsigned long timer1_get(){

    /*get global interrupt enable bit state*/
    unsigned char state = SREG;

    /*diable interrupts*/
    __builtin_avr_cli();

    /*get the count value*/
    unsigned long value = count;

    /*restore global interrupt state*/
    SREG |= state;

    /*return the count value*/
    return value;
}

/*--------------------------------------------------------
* timer1_clear
*
* Resets the seconds counter to zero.
*
* Notes:
* - Operation is interrupt-safe
--------------------------------------------------------*/
void timer1_clear(){
    /*get global interrupt enable bit state*/
    unsigned char state = SREG;

    /*disable interrupts*/
    __builtin_avr_cli();

    /*clear count*/
    count = 0;

    /*restore global interrupt state*/
    SREG |= state;
}

/*--------------------------------------------------------
* __vector_11 (Timer1 Compare Match ISR)
*
* Invoked once per second.
*
* - Increments the seconds counter
*
* Notes:
* - Minimal ISR design to reduce latency
--------------------------------------------------------*/
void __vector_11(void) __attribute__ ((signal, used, externally_visible));
void __vector_11(void){
    
    /*increment count by 1*/
    count++;
}
