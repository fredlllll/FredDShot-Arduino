/*
 * dshotDef.cpp
 *
 *  Created on: 04.07.2022
 *      Author: Frederik Gelder
 */

#include "dshotdef.h"

dshot::dShotData dshot::controllers[DSHOT_CONTROLLER_COUNT];

void dshot::sendController(const dShotData data) {
	uint8_t pinPort = data.pinPort;
	if (pinPort == 0) {
		return; //dont process uninitialized
	}
	uint8_t pinMaskOn = data.pinMaskOn;
	uint8_t pinMaskOff = data.pinMaskOff;
	bool *bits = data.bits;

	cli(); // dont need other timer interrupts or so messing with my output
	for (int i = 0; i < 16; ++i) {
		*((volatile uint8_t*) pinPort) |= pinMaskOn;
		//TODO: adjust nop counts depending on how many cycles instructions and loop take
		if (bits[i]) {
			//high bit
			nop<T1H>();
			*((volatile uint8_t*) pinPort) &= pinMaskOff;
			nop<T1L>();
		} else {
			//low bit
			nop<T0H>();
			*((volatile uint8_t*) pinPort) &= pinMaskOff;
			nop<T0L>();
		}
	}
	sei();
}

void dshot::sendData() {
	for (int i = 0; i < DSHOT_CONTROLLER_COUNT; ++i) {
		sendController(controllers[i]);
	}
}

const uint8_t dshot::calcCrc(const uint16_t value) {
	//https://github.com/JyeSmith/dshot-esc-tester/blob/master/dshot-esc-tester.ino#L403
	//https://github.com/betaflight/betaflight/blob/09b52975fbd8f6fcccb22228745d1548b8c3daab/src/main/drivers/pwm_output.c#L523
	int csum = 0;
	int csum_data = value;
	for (int i = 0; i < 3; i++) {
		csum ^= csum_data;
		csum_data >>= 4;
	}
	csum &= 0xf;
	return csum;
}

const uint16_t dshot::createFrame(uint16_t throttle, const bool telemetry =
		false) {
	throttle &= 0x7FF;
	uint16_t frame = throttle << 1;
	if (telemetry) {
		frame |= 1;
	}

	//uint8_t crc = (frame ^ (frame >> 4) ^ (frame >> 8)) & 0x0F;
	uint8_t crc = calcCrc(frame);
	frame <<= 4;
	frame |= crc;
	return frame;
}
