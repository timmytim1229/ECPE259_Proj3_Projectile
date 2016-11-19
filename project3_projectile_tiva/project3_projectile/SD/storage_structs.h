#ifndef STORAGE_STRUCTS_H_
#define STORAGE_STRUCTS_H_

#include <stdint.h>

struct __attribute__((__packed__)) radioPacket {
	uint8_t startByte;

	uint16_t status;
	uint16_t batteryVoltage;
	uint16_t batteryCurrent;
	int32_t rtc;

	float latitude;
	float longitude;
	uint16_t height;
	uint16_t heading;
	uint16_t speed;

	int16_t tempInt;
	int16_t tempExt;
	uint16_t humidity;

	uint8_t stopByte;
} typedef RadioPacket;

struct __attribute__((__packed__)) dataPacket {
	uint8_t startByte;

	uint16_t status;
	int32_t rtc;

	float latitude;
	float longitude;
	float height;
	float heading;
	float speed;
	uint8_t fix;
	uint8_t sats;

	int16_t tempInt;
	int16_t tempExt;
	uint16_t humidity;
	uint16_t pressure;

	int16_t accX;
	int16_t accY;
	int16_t accZ;
	int16_t gyroX;
	int16_t gyroY;
	int16_t gyroZ;
	float mag;

	uint16_t batteryVoltage;
	uint16_t batteryCurrent;
	uint32_t energyUsed;

	uint16_t extVoltage;
	uint16_t extCurrent;
	uint32_t extEnergyUsed;

	uint8_t stopByte;
} typedef DataPacket;

#endif
