#include "batt.h"

// Initialize port and ADC
void battery_setup( void ) {
	analogReference(INTERNAL);
	pinMode(BATT_IO, OUTPUT);
	digitalWrite(BATT_IO, HIGH); // Disable FET
	analogRead(BATT_ADC); // ADC dummy read
}

// 1,1V = 1024, Voltage divider: 4,7K / 1,0K
// Voltage reference in ATMega328P is 1,1V -> 6,27V in = 1024
// Calculation in millivolts (1V -> 1000mV)
// Pin 7 is triggered to enable voltage measurement FET
uint16_t battery_getvoltage( void ) {
	uint32_t value = 0;
	digitalWrite(BATT_IO, LOW); // Enable FET
	value = analogRead(BATT_ADC);
	digitalWrite(BATT_IO, HIGH); // Disable FET
	return (value*1100*5.7)/1024;
}
