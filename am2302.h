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
#include <plib.h> // Include the PIC32 Peripheral Library
void initTemperatureHumidity(IoPortId port, unsigned int pin);
char readTemperatureHumidity(IoPortId port, unsigned int pin, char timerId, short *temperature, short *humidity);
