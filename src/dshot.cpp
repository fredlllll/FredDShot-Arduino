/*
 * dshot.cpp
 *
 *  Created on: 04.07.2022
 *      Author: Frederik Gelder
 */

#include "dshot.h"

void setDShotThrottle(const uint8_t controller, const uint16_t throttle,
		const bool telemetry = false) {
	uint16_t frame = dshot::createFrame(throttle, telemetry);
	for (int i = 15, j = 0; i >= 0; --i, ++j) {
		uint8_t _bit = (frame >> i) & 1;
		dshot::controllers[controller].bits[j] = _bit != 0;
	}
}

void setDShotThrottle(const uint16_t throttle, const bool telemetry = false) {
	setDShotThrottle(0, throttle, telemetry);
}

void initDShot() {
	// zero bit buffers
	for (int i = 0; i < DSHOT_CONTROLLER_COUNT; ++i) {
		dshot::controllers[i].pinPort = 0;
		dshot::controllers[i].pinMaskOn = 0;
		dshot::controllers[i].pinMaskOff = 0;
		setDShotThrottle(i, 48, false);
	}

	// init timer1
	// source: http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
	// TIMER 1 for interrupt frequency 1000 Hz:
	cli();// stop interrupts
	TCCR1A = 0; // set entire TCCR1A register to 0
	TCCR1B = 0; // same for TCCR1B
	TCNT1 = 0; // initialize counter value to 0
	// set compare match register for 1000 Hz increments
	// OCR1A = 15999; // = 16000000 / (1 * 1000) - 1 (must be <65536)
	OCR1A = (F_CPU / DSHOT_UPDATE_FREQUENCY) - 1;
	// turn on CTC mode
	TCCR1B |= (1 << WGM12);
	// Set CS12, CS11 and CS10 bits for 1 prescaler
	TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
	// enable timer compare interrupt
	TIMSK1 |= (1 << OCIE1A);
	sei(); // allow interrupts
}

ISR(TIMER1_COMPA_vect) {
	dshot::sendData();
}

void setDShotPin(const uint8_t controller, const uint8_t pin) {
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
	dshot::controllers[controller].pinPort = dshot::constDigitalPinToPort(pin);
	auto pinIndex = dshot::constDigitalPinToIndex(pin);
	dshot::controllers[controller].pinMaskOn = 1 << pinIndex;
	dshot::controllers[controller].pinMaskOff = ~(1 << pinIndex);
}

void setDShotPin(const uint8_t pin) {
	setDShotPin(0, pin);
}
