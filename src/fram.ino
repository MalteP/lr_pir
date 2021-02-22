#include <SPI.h>
#include <Adafruit_FRAM_SPI.h>
#include <FastCRC.h>
#include "fram.h"

Adafruit_FRAM_SPI *fram = NULL;
bool fram_found = false;
FastCRC8 CRC8;

// Initialize FRAM
bool fram_setup( void ) {
	uint8_t statusRegister;

	// Check SPI bus for any known FRAM chip
	fram = new Adafruit_FRAM_SPI(FRAM_CS);
	fram_found = fram->begin();
	if(fram_found==false) {
		return false;
	}

	// Check status register for active write protection
	statusRegister = fram->getStatusRegister();
	if(statusRegister&((1<<FRAM_WPEN)|(1<<FRAM_BP1)|(1<<FRAM_BP0))) {
		statusRegister &= ~((1<<FRAM_WPEN)|(1<<FRAM_BP1)|(1<<FRAM_BP0));
		fram->writeEnable(true);
		fram->setStatusRegister(statusRegister);
		fram->writeEnable(false);
		Serial.println("FRAM write protection has been disabled");
	}

	return true;
}

// Read FRAM data and validate CRC8 checksum
bool fram_read( void *data, size_t length ) {
	uint8_t crc, crc2;

	// FRAM initialized?
	if(fram_found==false) {
		return false;
	}

	// Read CRC from first byte, then read data
	fram->read(0, (uint8_t *)&crc, sizeof(uint8_t));
	fram->read(1, (uint8_t *)data, length);

	// Calculate and validate checksum
	crc2 = CRC8.smbus((uint8_t *)data, length);
	if(crc!=crc2) {
		Serial.println(F("FRAM checksum mismatch"));
		memset(data, 0, length);
		return false;
	}

	return true;
}

// Write CRC and data to FRAM
bool fram_write( void *data, size_t length ) {
	uint8_t crc;

	// FRAM initialized?
	if(fram_found==false) {
		return false;
	}

	// Calculate checksum and write data
	crc = CRC8.smbus((uint8_t *)data, length);
	fram->writeEnable(true);
	fram->write(0, (uint8_t *)&crc, sizeof(uint8_t));
	fram->writeEnable(false);
	fram->writeEnable(true);
	fram->write(1, (uint8_t *)data, length);
	fram->writeEnable(false);

	return true;
}
