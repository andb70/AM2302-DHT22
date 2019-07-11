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
#include "am2302.h"
#include "../timers1.X/timers.h"

void initTemperatureHumidity(IoPortId port, unsigned int pin){
    PORTSetPinsDigitalOut(port, pin);// switch to output
    PORTSetBits(port, pin);//host pull-low
}

char readTemperatureHumidity(IoPortId port, unsigned int pin, char timerId, short *temperature, short *humidity){
/*		Communication with sensor AM2302 require this timing						
 *                                      us      Cycles	THS     THS Cycles	
 *		cycle time		8				*			
 *          Step 0: wait before next request 				
 *	SLEEP           wait 2 seconds   2000000	250000		
 *	                Pin is input, wire is high level	
 * 					
 *          Step 1: switch wire to output
 *                  MCU send out start signal to AM2302						
 *	SEND_START      Pin set as OUTPUT						
 *	SEND_LO         host pull-low		3200	400			
 *	SEND_HI         host pull-up		32      4	
 * 
 *          Switch wire to input		
 *                  AM2302 send response signal to MCU
 *                  if wait time exceeds threshold: error      
 *  SENSOR_WAKE     **NOT DOCUMEMTED** 	40    sensor pulls the line low				
 *	WAIT_REPLY      Pin set as INPUT						
 *	WAIT_LOW        sensor pull-low		80      10      120     15	
 *	WAIT_UP         sensor pull-up		80      10      120     15	
 * 
 *          Step 2: AM2302 send data to MCU				
 *                  Sensor sends 40 bits						
 *                  if wait time exceeds threshold: error 
 *	WAIT_START      start transmit		50      6.25	72      9	
 *	WAIT_BIT        sig = 0             28      3.5     50      6.25	
 *	WAIT_BIT        sig = 1             70      8.75	50      6.25
 * 	
 *			Step 3: check the received message					
 *	CHECK_DATA	
 *  LAST_BIT        sensor pulls Hi in  48
 *								
 */								

    #define readBit()       (PORTReadBits(port, pin)>>pin)

    #define SENSOR_IDLE         (4000000L)
    #define HOST_PULL_LOW       (3200L)
    #define HOST_PULL_UP        32
    #define SENSOR_WAKE         40
    #define SENSOR_PULL_THS     90
    #define SENSOR_PULL_DIFF    20
    #define START_TRANSMIT_THS  80
    #define TRANSMIT_FALSE_THS  48
    #define TRANSMIT_TRUE_THS   48
    #define TRANSMIT_THS        16

    static enum _pState{
        POR,
        sleep,
        sendLow,
        sendHi,
        waitReply,
        waitLow,
        waitHi,
        waitStart,
        waitBit,
        waitBitHi,
        storeData,
        checkData,
        transmissionEnd,
        trransmissionError
    } pState;
    
    static union _am2302{
        struct {
            unsigned humidity: 16;
            unsigned temperature: 15;
            unsigned sign: 1;
            unsigned crc: 8;  
            unsigned bytePos: 4;
            unsigned bitPos: 4;
        };
        unsigned char bytes[6];
    } am2302;
    char amPos[]={1,0,3,2,4};// XX-Endian conversion: swap low and high byte

    switch(pState)
    {
        case POR:
            if(timer_usFinished(timerId))
            {
                timer_usSet(timerId, SENSOR_IDLE);
                pState = sleep;
            }
            break;
            
        case sleep:
            if(timer_usFinished(timerId))
            {
                // Step 1: MCU send out start signal to AM2302
                PORTSetPinsDigitalOut(port, pin); // switch to output
                PORTClearBits(port, pin); //host pull-low
                timer_usSet(timerId, HOST_PULL_LOW);
                pState = sendLow;
            }
            break;
            
        case sendLow:
            if(timer_usFinished(timerId))
            {     
                // first period at low voltage done
                PORTSetBits(port, pin); //host pull-hi
                timer_usSet(timerId, HOST_PULL_UP);
                pState = sendHi;
            }
            break;  
            
        case sendHi:
            if(timer_usFinished(timerId))
            {      
                // second period at high voltage done
                // now set the port as input
                // and wait few us for the commutation
                PORTSetPinsDigitalIn(port, pin);// switch to input
                timer_usSet(timerId, SENSOR_WAKE);
                pState = waitReply;
            }
            break; 
            
        case waitReply:
            if(timer_usFinished(timerId))
            {    
                pState = trransmissionError;
                break;
            }      
            if ( !readBit())
            {
                // commutation ok, wait third period put low by sensor
                // AM2302 send response signal to MCU,
                // if threshold exceeds: error  
                timer_usSet(timerId, SENSOR_PULL_THS );
                pState = waitLow;
            }
            break;  
            
        case waitLow:
            if(timer_usFinished(timerId))
            {      
                // third period > 90us 
                pState = trransmissionError;
                break;
            }
            if ( ! readBit())
                break;
            // third period low by sensor done
            // check if it was < 70 us
            if (timer_usElapsed(timerId)<SENSOR_PULL_THS - SENSOR_PULL_DIFF)
            {
                pState = trransmissionError;
            }
            // sensor has put the line up, wait for the falling front
            timer_usSet(timerId, SENSOR_PULL_THS);
            pState = waitHi;            
            break;  
            
        case waitHi:
            if(timer_usFinished(timerId))
            {    
                // fourth period > 90us
                pState = trransmissionError;
                break;
            }
            if ( readBit())
                break;
            // fourth period high by sensor done            
            // check if it was < 70 us
            if (timer_usElapsed(timerId)<SENSOR_PULL_THS - SENSOR_PULL_DIFF)
            {
                pState = trransmissionError;
            }            // sensor has put the line low, reset the buffer and position
            am2302.bytes[0] = 0; // humidity LOW
            am2302.bytes[1] = 0; // humidity HIGH
            am2302.bytes[2] = 0; // temperature LOW
            am2302.bytes[3] = 0; // temperature HI-15b / sign -Msb
            am2302.bytes[4] = 0; // CRC
            am2302.bytes[5] = 0x70;//0x74;// bytePos = 4, bitPos = 7           
            timer_usSet(timerId, START_TRANSMIT_THS);
            pState = waitStart;
            break;  
            
        case waitStart:
            // the sensor is writing the start Transmit 50 us Low
            if(timer_usFinished(timerId))
            {       
                pState = trransmissionError;
                break;
            }
            // we wait for the sensor to put the line up
            if (! readBit())
                break;
            // sensor has put the line up, star measuring the duration           
            timer_usSet(timerId, TRANSMIT_FALSE_THS);
            pState = waitBit;
            break;
            
        case waitBit:            
            // the sensor is writing the DATA 28..70 us High
            if(timer_usFinished(timerId))
            {      
                // duration for FALSE is over, let's see if it is TRUE
                timer_usSet(timerId, TRANSMIT_TRUE_THS);
                pState = waitBitHi;
                break;
            }
            if (readBit())
                break;
            // sensor has put the line down, the bit is valid and it's FALSE     
            // we don't need to store the value, simply increment position
            pState = storeData;
            break;
            
        case waitBitHi:            
            // the sensor is writing the DATA ..70 us High
            if(timer_usFinished(timerId))
            {      
                // duration for TRUE is over, transmission is broken   
                pState = trransmissionError;
                break;
            }
            if (readBit())
                break;
            // sensor has put the line down, the bit is valid and it's TRUE     
            // store the value
            // put the bit in the right place
            am2302.bytes[amPos[am2302.bytePos]] |= 1<<am2302.bitPos;          
            pState = storeData;
            
        case storeData:
            // check if it's end of transmission
            if (am2302.bitPos--==0)
            {
                am2302.bitPos = 7;
                if (++am2302.bytePos>4)
                {                    
                    pState = checkData;
                    break;
                }                
            }            

            // not yet, wait another bit
            timer_usSet(timerId, START_TRANSMIT_THS);
            pState = waitStart;
            break;          
            
        case checkData: // Step 3: check the received message
            am2302.bytes[5] =am2302.bytes[0] +am2302.bytes[1] +am2302.bytes[2] +am2302.bytes[3];
            if (am2302.bytes[5] != am2302.bytes[4]){
                pState = trransmissionError;
                break;
            }

            if (am2302.sign)
                *temperature = -am2302.temperature;
            else
                *temperature = am2302.temperature;
            *humidity = am2302.humidity;
            pState = transmissionEnd;
            return 1;
            
        case transmissionEnd:
            timer_usSet(timerId, SENSOR_IDLE);
            pState = sleep;
            break;
            
        case trransmissionError:
            // manage errors
            pState = transmissionEnd;
            break;            
    }
    return 0;
}
