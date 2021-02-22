#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "setup.h"

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static uint8_t sending = 0;

void onEvent (ev_t ev) {
	switch(ev) {
		case EV_SCAN_TIMEOUT:
			Serial.println(F("EV_SCAN_TIMEOUT"));
			break;
		case EV_BEACON_FOUND:
			Serial.println(F("EV_BEACON_FOUND"));
			break;
		case EV_BEACON_MISSED:
			Serial.println(F("EV_BEACON_MISSED"));
			break;
		case EV_BEACON_TRACKED:
			Serial.println(F("EV_BEACON_TRACKED"));
			break;
		case EV_JOINING:
			Serial.println(F("EV_JOINING"));
			break;
		case EV_JOINED:
			Serial.println(F("EV_JOINED"));
			break;
		case EV_RFU1:
			Serial.println(F("EV_RFU1"));
			break;
		case EV_JOIN_FAILED:
			Serial.println(F("EV_JOIN_FAILED"));
			break;
		case EV_REJOIN_FAILED:
			Serial.println(F("EV_REJOIN_FAILED"));
			break;
		case EV_TXCOMPLETE:
			Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
			if (LMIC.txrxFlags & TXRX_ACK) {
				Serial.println(F("Received ack"));
			}
			if (LMIC.dataLen) {
				Serial.println(F("Received "));
				Serial.println(LMIC.dataLen);
				Serial.println(F(" bytes of payload"));
			}
			sending = 0;
			break;
		case EV_LOST_TSYNC:
			Serial.println(F("EV_LOST_TSYNC"));
			break;
		case EV_RESET:
			Serial.println(F("EV_RESET"));
			break;
		case EV_RXCOMPLETE:
			// data received in ping slot
			Serial.println(F("EV_RXCOMPLETE"));
			break;
		case EV_LINK_DEAD:
			Serial.println(F("EV_LINK_DEAD"));
			break;
		case EV_LINK_ALIVE:
			Serial.println(F("EV_LINK_ALIVE"));
			break;
		case EV_SCAN_FOUND:
			Serial.println(F("EV_SCAN_FOUND"));
			break;
		case EV_TXSTART:
			Serial.println(F("EV_TXSTART"));
			break;
		case EV_TXCANCELED:
			Serial.println(F("EV_TXCANCELED"));
			break;
		case EV_RXSTART:
			Serial.println(F("EV_RXSTART"));
			break;
		case EV_JOIN_TXCOMPLETE:
			Serial.println(F("EV_JOIN_TXCOMPLETE"));
			break;
		 default:
			Serial.println(F("Unknown event"));
			break;
	}
}

void ttn_setup( void ) {
	// LMIC init
	os_init();
	// Reset the MAC state. Session and pending data transfers will be discarded.
	LMIC_reset();

	// Set static session parameters. Instead of dynamically establishing a session
	// by joining the network, precomputed session parameters are be provided.
	#ifdef PROGMEM
	// On AVR, these values are stored in flash and only copied to RAM
	// once. Copy them to a temporary buffer here, LMIC_setSession will
	// copy them into a buffer of its own again.
	uint8_t appskey[sizeof(APPSKEY)];
	uint8_t nwkskey[sizeof(NWKSKEY)];
	memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
	memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
	LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
	#else
	// If not running an AVR with PROGMEM, just use the arrays directly
	LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
	#endif

	#if defined(CFG_eu868)
	// Set up the channels used by the Things Network, which corresponds
	// to the defaults of most gateways. Without this, only three base
	// channels from the LoRaWAN specification are used, which certainly
	// works, so it is good for debugging, but can overload those
	// frequencies, so be sure to configure the full frequency range of
	// your network here (unless your network autoconfigures them).
	// Setting up channels should happen after LMIC_setSession, as that
	// configures the minimal channel set.
	// NA-US channels 0-71 are configured automatically
	LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);	  // g-band
	LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);	  // g-band
	LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);	  // g-band
	LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);	  // g-band
	LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);	  // g-band
	LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);	  // g-band
	LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);	  // g-band
	LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);	  // g-band
	LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);	  // g2-band
	// TTN defines an additional channel at 869.525Mhz using SF9 for class B
	// devices' ping slots. LMIC does not have an easy way to define set this
	// frequency and support for class B is spotty and untested, so this
	// frequency is not configured here.
	#elif defined(CFG_us915)
	// NA-US channels 0-71 are configured automatically
	// but only one group of 8 should (a subband) should be active
	// TTN recommends the second sub band, 1 in a zero based count.
	// https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
	LMIC_selectSubBand(1);
	#endif

	// Disable link check validation
	LMIC_setAdrMode(0);
	LMIC_setLinkCheckMode(0);

	#ifdef SINGLE_CH_GATEWAY
	// If using a mono-channel gateway disable all channels
	// but the one the gateway is listening to
	//LMIC_disableChannel(0);
	LMIC_disableChannel(1);
	LMIC_disableChannel(2);
	LMIC_disableChannel(3);
	LMIC_disableChannel(4);
	LMIC_disableChannel(5);
	LMIC_disableChannel(6);
	LMIC_disableChannel(7);
	LMIC_disableChannel(8);
	#endif

	// TTN uses SF9 for its RX2 window.
	LMIC.dn2Dr = DR_SF9;

	// Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
	LMIC_setDrTxpow(TX_DATARATE, 14);
}

void ttn_shutdown( void ) {
	// Shutdown MAC activity
	LMIC_shutdown();
}

// Save sequence numbers before sleep mode
void ttn_getseq( uint32_t *mySeqUp, uint32_t *mySeqDn ) {
	*mySeqUp = LMIC.seqnoUp;
	*mySeqDn = LMIC.seqnoDn;
}

// Restore sequence numbers after sleep mode
void ttn_setseq( uint32_t mySeqUp, uint32_t mySeqDn ) {
	LMIC.seqnoUp = mySeqUp;
	LMIC.seqnoDn = mySeqDn;
}

void ttn_send( uint8_t *mydata, uint8_t mysize ) {
	// Check if there is not a current TX/RX job running
	if (LMIC.opmode & OP_TXRXPEND) {
		Serial.println(F("OP_TXRXPEND, not sending"));
	} else {
		// Prepare upstream data transmission at the next possible time.
		LMIC_setTxData2(1, mydata, mysize, 0);
		Serial.println(F("Packet queued"));
		sending = 1;
	}
}

uint8_t ttn_sending( void ) {
	return sending;
}

void ttn_loop( void ) {
	os_runloop_once();
}
