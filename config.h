#ifndef _CONFIG
#define _CONFIG

// System clock frequency
#define SYS_FREQ 			(80000000L)


/******************************************************************************/
// TIMERS
// timers management
//      IF we let compile time pre-processor calculate the PR (period)
//          PR = Freq_clk/freq/PBDIV/PS-1
//          PR = SYS_FREQ/PRESCALE/TMR_FREQ-1
// TIMER 1
// use TIMER 1?
#define USE_TIMER1          1
#if USE_TIMER1
#define T1_EN               T1_ON
#else
#define T1_EN               T1_OFF
#endif
#define T1_SOURCE           T1_SOURCE_INT
#define T1_INT              T1_INT_ON
#define T1_INT_PRIOR        T1_INT_PRIOR_2

// desired frequency: 1 kHz
//            period: 1 ms
#define TMR_FREQ_1          (1000L)
#define PBDIV_1             1
#define PRESCALE_1          T1_PS_1_64
#define PS_1                64
//#define PR_1                0x4E1
#define PR_1                (SYS_FREQ/TMR_FREQ_1/PBDIV_1/PS_1-1)

// TIMER 2
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
//      62.5 kHz    4                   16 us  use up to 16 timers
//      ...
//      15625 Hz    6   last integer    64 us 
//      ...
//      976.5625 Hz 10               about 1 ms 
//      ...
//
// max timer duration is about   1h 11' 34''
//
/******************************************************************************/
#define TMR_FREQ_K          3
// from 0 to 13 instances under TIMER 2
#define NUM_TIMERS_us       1

// desired frequency: 125 kHz
//            period: 8 us
//#define TMR_FREQ_2          (125000L)
// desired frequency: 62.5 kHz
//            period: 16 us
#define TMR_FREQ_2          (1000000L>>TMR_FREQ_K)
#define PBDIV_2             1
#define PRESCALE_2          T2_PS_1_1
#define PS_2                1
//#define PR_2                0x27F
#define PR_2                (SYS_FREQ/TMR_FREQ_2/PBDIV_2/PS_2-1)

#endif