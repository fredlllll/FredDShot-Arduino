/*
 * dshotDef.cpp
 *
 *  Created on: 04.07.2022
 *      Author: Frederik Gelder
 */

#include "dshotdef.h"

dshot::dShotData dshot::controllers[DSHOT_CONTROLLER_COUNT];

__attribute__((noinline)) void dshot::sendController(const dShotData &data) {
	uint8_t pinPort = data.pinPort;
	if (pinPort == 0) {
		return; //dont process uninitialized
	}
	uint8_t pinMaskOn = data.pinMaskOn;
	uint8_t pinMaskOff = data.pinMaskOff;
	bool *bits = data.bits;

	cli(); // dont need other timer interrupts or so messing with my output

	//i painstakingly counted the cycles from the disassembly of this
	//an assembly block would indeed be better suited for this
	//but assembly is very hard to understand (and write interop with c++)
	for (uint8_t i = 0; i < 16; ++i) { //ldi	r31, 0x00	; 0
		//we have to switch on in both cases, so might aswell do it here
		*((volatile uint8_t*) pinPort) |= pinMaskOn; // 2(ld) + 1(or) + 2(st) = 5 cycles
		//TODO: adjust nop counts depending on how many cycles instructions and loop take
		if (bits[i]) { // ld, and, brne, rjmp. 2+1+1+2 if low bit, 2+1+2 if high bit
			//high bit
			nop<T1H - 5 - 5>();	//subtract the following cycles till pin is switched off and the comparison+jmp
			*((volatile uint8_t*) pinPort) &= pinMaskOff; // 2(ld) + 1(and) + 2(st) = 5 cycles
			nop<T1L - 5 - 6>(); //subtract the following cycles till switch on is finished again
			//ldi, cpi, cpc, breq, rjmp- 1+1+1+1+2=6 if not end of loop, 1+1+1+2=5 if end of loop
		} else {
			//low bit
			nop<T0H - 5 - 6>(); //subtract the following cycles till pin is switched off and the comparison+jmp
			*((volatile uint8_t*) pinPort) &= pinMaskOff; // 2(ld) + 1(and) + 2(st) = 5 cycles
			nop<T0L - 5 - 6 - 2>(); //subtract the following cycles till switch on is finished again
			//rjmp. 2 right into the nops at the end of high bit part
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
