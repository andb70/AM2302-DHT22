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
#ifndef _TIMERS
#define _TIMERS
#include <p32xxxx.h> // Include PIC32 specifics header file
#include "config.h"

// timers management
/******************************************************************************/
// timer1 config is given by config.h
/******************************************************************************/
// timer2
//      IF we let compile time pre-processor calculate the PR (period)
//          PR = Freq_clk/freq/PBDIV/PS-1
//          PR = SYS_FREQ/PRESCALE/TMR_FREQ-1
//




// timer management: for each timer, flag is set when the interval is elapsed
union _timerFlags{
    struct {
        unsigned t0 : 1;
        unsigned t1 : 1;
        unsigned t2 : 1;
        unsigned t3 : 1;
        unsigned t4 : 1;
        unsigned t5 : 1;
        unsigned t6 : 1;
        unsigned t7 : 1;
        unsigned t8 : 1;
        unsigned t9 : 1;
        unsigned t10 : 1;
        unsigned t11 : 1;
        unsigned t12 : 1;
        unsigned t13 : 1;
        unsigned t14 : 1;
        unsigned t15 : 1;
        unsigned t16 : 1;
        unsigned t17 : 1;
        unsigned t18 : 1;
        unsigned t19 : 1;
        unsigned t20 : 1;
        unsigned t21 : 1;
        unsigned t22 : 1;
        unsigned t23 : 1;
        unsigned t24 : 1;
        unsigned t25 : 1;
        unsigned t26 : 1;
        unsigned t27 : 1;
        unsigned t28 : 1;
        unsigned timer2 : 1;
        unsigned timer1 : 1;
        unsigned hertbeat : 1;
    };
    struct {
        unsigned w : 32;
    };
} timerFlags;
unsigned int timerIntervals[NUM_TIMERS_us];

// trigger the timer 1 
// and it's set to 1 only once between two calls to checkTimers()
#define timer1Status()          timerFlags.timer1 
// trigger the timer 2 at 1 Hz frequency 
// and it's set to 1 only once between two calls to checkTimers()
#define heartbeat()             timerFlags.hertbeat
// trigger the timer_us(id) interval
// and it's set to 1 until the interval elapses
#define timer_usRunning(id)     (timerFlags.w >> id & 0x01)
// trigger the timer_us(id) interval
// and it's set to 1 after the interval is elapsed
#define timer_usFinished(id)    (!(timerFlags.w >> id & 0x01))
#define timer_usElapsed(id)     (timerIntervals[id]<<TMR_FREQ_K)
void initTimers();

/******************************************************************************/
// call checkTimers() before any other function in the main loop, this will
// warranty that the status of any timer won't change before the next execution
// of checkTimers() and, consequently, different calls to
//        timer1Status()
//        heartbeat()
//        timer_usStatus(id)
//        timer_usElapsed(id)
// will give the same results between two consecutive calls to checkTimers()
// independently of the number of calls
void checkTimers();


/******************************************************************************/
//
//  timer_usSet(id, interval)
//              id: 0..29
//                  interval: timer duration in us
//
// about timer_usSet implementation:
//
// HINT: the timer resoution depends on the timer frequency 
//
// a timer in us should run at 1MHz but usually it runs at a lower frequency 
// so the timer convert the interval from us to timer unit, proportionally to
// TMR_FREQ and store the converted value
//
// the conversion factor is K = 1 [MHz] / TMR_FREQ [MHz]
// or TMR_FREQ = 10^6 [Hz]   *   2^-K   so valid values for
//     TMR_FREQ and K are:          resolution 
//      1 MHz       0                   1 us  too high
//      500 kHz     1                   2 us  too high
//      250 kHz     2                   4 us  use only 1 timer
//      125 kHz     3                   8 us  use up to 8 timers
//      62.5 kHz    4                   16 us  use up to 13 timers
//      ...
//      15625 Hz    6   last integer   64 us 
//      ...
//      976 Hz      10  (976.5625)   about 1 ms 
//      ...
//
// max timer duration is about   1h 11' 34''
//
/******************************************************************************/
void timer_usSet(char id, unsigned int interval);
// use timer_msSet or timer_sSet if you're lazy and don't want to write 
// large numbers when you need intervals in ms or seconds
#define timer_msSet(id, interval)   timer_usSet(id, interval*1000)
#define timer_sSet(id, interval)   timer_usSet(id, interval*1000000)

#endif