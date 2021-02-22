#ifndef SETUP_H
#define SETUP_H

#include "led.h"

// LoRaWAN NwkSKey, network session key
static const PROGMEM u1_t NWKSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// LoRaWAN AppSKey, application session key
static const u1_t PROGMEM APPSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// LoRaWAN end-device address (DevAddr)
static const u4_t DEVADDR = 0x00000000;

// Uncomment if using a single channel gateway
//#define SINGLE_CH_GATEWAY

// TX datarate
#define TX_DATARATE DR_SF7

// Minimum TX duty cycle in seconds
const unsigned TX_INTERVAL = 120;

// Choose LED mode on alarm (LED_OFF, LED_ON, LED_BLINK)
#define LED_MODE LED_BLINK

// Pin mapping
const lmic_pinmap lmic_pins = {
	.nss = 10,
	.rxtx = LMIC_UNUSED_PIN,
	.rst = 6,
	.dio = {2, 4, 5},
};

#endif
