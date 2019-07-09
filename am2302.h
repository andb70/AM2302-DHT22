#include <plib.h> // Include the PIC32 Peripheral Library
void initTemperatureHumidity(IoPortId port, unsigned int pin);
char readTemperatureHumidity(IoPortId port, unsigned int pin, char timerId, short *temperature, short *humidity);
