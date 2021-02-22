#include <lmic.h>
#include <hal/hal.h>
#include "setup.h"

// Main state machine
enum { STATE_BOOT, STATE_INIT, STATE_SEND, STATE_SENDING, STATE_SHUTDOWN, STATE_DUTYCYCLE, STATE_SLEEP };

// Structure for sequence counters in FRAM
struct fram_data_t {
	uint32_t mySeqUp;
	uint32_t mySeqDn;
};

// Different variables used in main loop
static uint8_t state = STATE_BOOT;
static uint8_t mydata[4];
static uint16_t batt = 0;
static struct fram_data_t fram_data;

// System initialization
void setup() {
	Serial.begin(115200);
	Serial.println(F("System starting"));
	memset(&fram_data, 0, sizeof(struct fram_data_t));
}

// Main loop
void loop() {
	switch(state) {
		case STATE_BOOT: // Entered at boot for some basic initialization before going into sleep mode
			ttn_setup();
			ttn_shutdown();
			led_setup();
			fram_setup();
			fram_read(&fram_data, sizeof(struct fram_data_t));
			state = STATE_SLEEP;
			break;
		case STATE_INIT: // Initialization after low power sleep mode
			battery_setup();
			ttn_setup();
			ttn_setseq(fram_data.mySeqUp, fram_data.mySeqDn);
			++state;
			break;
		case STATE_SEND: // Send data packet
			led_enable(LED_MODE);
			batt = battery_getvoltage();
			mydata[0] = 'A';			// Alarm
			mydata[1] = 0;				// Input 0
			mydata[2] = (batt>>8);		// Battery voltage MSB
			mydata[3] = (batt&0xFF);	// Battery voltage LSB
			Serial.println(F("Send packet"));
			ttn_send(mydata, sizeof(mydata));
			++state;
			break;
		case STATE_SENDING: // Loop while packet is queued
			if(ttn_sending()==0) {
				++state;
			}
			break;
		case STATE_SHUTDOWN: // Shutdown LMIC and save counters
			ttn_shutdown();
			ttn_getseq(&fram_data.mySeqUp, &fram_data.mySeqDn);
			fram_write(&fram_data, sizeof(struct fram_data_t));
			led_disable();
			Serial.flush();
			++state;
			break;
		case STATE_DUTYCYCLE: // Enforce LoRa duty cycle by entering low power sleep mode for a time defined in setup.h
			Serial.println(F("Duty cycle"));
			Serial.flush();
			lowpower_sleep_interval();
			++state;
			break;
		case STATE_SLEEP: // Enter low power sleep mode until external interrupt pin is triggered
			Serial.println(F("Sleep mode"));
			Serial.flush();
			lowpower_sleep_interrupt();
			Serial.println(F("Wake up"));
			state = STATE_INIT;
			break;
	}

	// Must be called within the main loop
	ttn_loop();
	led_loop();
}
