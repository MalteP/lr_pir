#include <LowPower.h>
#include "setup.h"

// Low power sleep mode for a defined time, used to enforce TX duty cycle
void lowpower_sleep_interval( void ) {
	uint32_t cycles = TX_INTERVAL/8;

	// Sleep given duty cycle
	do {
		LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
	} while(--cycles>0);
}

// Low power sleep mode until interrupt pin is triggered
void lowpower_sleep_interrupt( void ) {
	// Allow wake up pin to trigger interrupt on rising edge.
	attachInterrupt(1, lowpower_wakeUp, RISING);

	// Enter power down state with ADC and BOD module disabled.
	// Wake up when wake up pin is high.
	LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 

	// Disable external pin interrupt on wake up pin.
	detachInterrupt(1);
}

// Wake up interrupt
void lowpower_wakeUp( void ) {
	// Just a handler for the pin interrupt.
}
