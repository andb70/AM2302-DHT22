/*
 * released under GNU AFFERO GENERAL PUBLIC LICENSE Version 3, 19 November 2007
 * 
 *      Author:     Andrea Biasutti
 *      Date:       July 5th 2019
 *      Hardware:   PIC32MX440256H
 * 
 * source available here https://github.com/andb70/Timers.X
 * 
 */
#include <p32xxxx.h> // Include PIC32 specifics header file
#include <plib.h> // Include the PIC32 Peripheral Library
#include "timers.h"

union _timerElapsed {
    struct {
        unsigned timer1 : 1;
        unsigned timer2 : 1;
        unsigned : 6;
    };
    struct {
        unsigned w : 8;
    };
} timerNotify;

// heartbeat base pulsation is under timer2 scan frequency, hence in us
unsigned int heartbeatCount;

void initTimers() {
    // enable multi-vector interrupts
    INTEnableSystemMultiVectoredInt();
    // optimize pic32 performances
    SYSTEMConfigPerformance(SYS_FREQ);

    // configure Timer 1 using internal clock
    OpenTimer1(T1_EN | T1_SOURCE | PRESCALE_1, PR_1);
    // set up the timer interrupt with a priority of 2
    ConfigIntTimer1(T1_INT | T1_INT_PRIOR);

    // configure Timer 1 using internal clock
    OpenTimer2(T2_ON | T2_SOURCE_INT | PRESCALE_2, PR_2);
    // set up the timer interrupt with a priority of 2
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_2);
    
    // reset the flags and counter
    heartbeatCount = TMR_FREQ_2;
    timerNotify.w=0;
}

void __ISR(_TIMER_1_VECTOR, IPL2SOFT) Timer1Handler(void) {
    // set the notify bit
    timerNotify.timer1 = 1;
    // clear the interrupt flag
    mT1ClearIntFlag();
}
void __ISR(_TIMER_2_VECTOR, IPL2SOFT) Timer2Handler(void) {
    // set the notify bit
    timerNotify.timer2 = 1;
    // clear the interrupt flag
    mT2ClearIntFlag();
}

void checkTimers() {
    // sync the timer status to be available without change for the entire cycle

    timerFlags.timer1=0;
    if ( timerNotify.timer1)
    {
        timerNotify.timer1 = 0;
        timerFlags.timer1=1;
    }
    
    // reset the heartbeat
    timerFlags.hertbeat = 0;
    timerFlags.timer2=0;
    if (timerNotify.timer2)
    {
        timerNotify.timer2 = 0;
        timerFlags.timer2=1;

        if (--heartbeatCount == 0) {
            timerFlags.hertbeat = 1;
            heartbeatCount = TMR_FREQ_2;
        }

        // compute all the timers
        char i;
        for (i=0; i< NUM_TIMERS_us; i++)
        {
            // if the timer is counting
            if (timerFlags.w>>i & 0x01)
            {        
                // decrement and when reach 0 stop counting
                 if(--timerIntervals[i]==0)                
                    timerFlags.w &= ~(1<<i);            
            }
        }
    }

}


void timer_usSet(char id, unsigned int interval){
    // set the status bit
    timerFlags.w |= 1<<id;
    // reset the interval
    timerIntervals[id] = (interval>>TMR_FREQ_K);
}
