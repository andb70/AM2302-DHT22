/*
 * released under GNU AFFERO GENERAL PUBLIC LICENSE Version 3, 19 November 2007
 * 
 *      Author:     Andrea Biasutti
 *      Date:       July 5th 2019
 *      Hardware:   PIC32MX440256H
 * 
 * source available here https://github.com/andb70/AM2302-DHT22
 * 
 */
#pragma config FPLLIDIV = DIV_2 // PLL Input Divider (1x Divider)
#pragma config FPLLMUL = MUL_20 // PLL Multiplier (24x Multiplier)
#pragma config UPLLIDIV = DIV_2 // USB PLL Input Divider (12x Divider)
#pragma config UPLLEN = OFF // USB PLL Enable (Disabled and Bypassed)
#pragma config FPLLODIV = DIV_1 // System PLL Output Clock Divider (PLL Divide by 256)
// DEVCFG1
#pragma config FNOSC = PRIPLL // Oscillator Selection Bits (Primary Osc w/PLL (XT+,HS+,EC+PLL))
#pragma config FSOSCEN = ON // Secondary Oscillator Enable (Enabled)
#pragma config IESO = ON // Internal/External Switch Over (Enabled)
#pragma config POSCMOD = HS // Primary Oscillator Configuration (HS osc mode)
#pragma config OSCIOFNC = ON // CLKO Output Signal Active on the OSCO Pin (Enabled)
#pragma config FPBDIV = DIV_8 // Peripheral Clock Divisor (Pb_Clk is Sys_Clk/8)
#pragma config FCKSM = CSDCMD // Clock Switching and Monitor Selection (Clock Switch Disable, FSCM Disabled)
#pragma config WDTPS = PS1048576 // Watchdog Timer Postscaler (1:1048576)
#pragma config FWDTEN = OFF // Watchdog Timer Enable (WDT Disabled (SWDTEN Bit Controls))
// DEVCFG0
#pragma config DEBUG = OFF // Background Debugger Enable (Debugger is disabled)
#pragma config ICESEL = ICS_PGx2 // ICE/ICD Comm Channel Select (ICE EMUC2/EMUD2 pins shared with PGC2/PGD2)
#pragma config PWP = OFF // Program Flash Write Protect (Disable)
#pragma config BWP = OFF // Boot Flash Write Protect bit (Protection Disabled)
#pragma config CP = OFF // Code Protect (Protection Disabled)

#include <p32xxxx.h> // Include PIC32 specifics header file
#include <plib.h> // Include the PIC32 Peripheral Library
#include <stdio.h>
#include <stdlib.h>
#include "../timers1.X/timers.h"
#include "am2302.h"

// HEARTBEAT led: toggle every seconds
// on board YELLOW
#define heartbeatLedToggle()    LATDbits.LATD1 = !LATDbits.LATD1

// CON5 D8
#define StatusLedSet(state)     LATBbits.LATB13 = state
#define StatusLedToggle()       LATBbits.LATB13 = !LATBbits.LATB13
void initLeds();
void initGlobals();

int main() {

    // initialize timer service
    initTimers();
    // Initialize port pins to be used with leds
    initLeds();
    // initialize port pins to be used with leds
    initGlobals();
    
    short temperature, humidity;
    initTemperatureHumidity(IOPORT_B, BIT_14);
    
    while (1) {
        // MANDATORY: first call in the loop
        checkTimers();
        if(heartbeat()) // signal every second the pic is alive
        {
            heartbeatLedToggle();
        }
        // query the measures  based on AM2302 specifications
        // every time we have a reading toggle the led
        if(readTemperatureHumidity(IOPORT_B, BIT_14, 0, &temperature, &humidity))
            StatusLedToggle();
      
    }
    return 1;
}

/******************************************************************************/
// INITIALIZATION FUNCTIONS

// initialize ports used with leds
void initLeds() {
    mJTAGPortEnable(0); // Disable JTAG
    PORTSetPinsDigitalOut(IOPORT_B, BIT_13); // RED led @ D8
    PORTSetPinsDigitalOut(IOPORT_D, BIT_1); //  YELLOW led on-board
    PORTSetPinsDigitalOut(IOPORT_G, BIT_6); //  GREEN  led on-board    
}

void initGlobals() {
    // default state: all off
    LATB = 0;
    LATD = 0;
    LATG = 0;
}