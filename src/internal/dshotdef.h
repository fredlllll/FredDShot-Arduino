/*
 * dshotDef.h
 *
 *  Created on: 04.07.2022
 *      Author: Frederik Gelder
 */

#ifndef DSHOTDEF_H_
#define DSHOTDEF_H_
#include "Arduino.h"
#include <stdint.h>

// how many dshot outputs we want
#ifndef DSHOT_CONTROLLER_COUNT
#define DSHOT_CONTROLLER_COUNT 1
#endif

// at what frequency we want to update the outputs
#ifndef DSHOT_UPDATE_FREQUENCY
#define DSHOT_UPDATE_FREQUENCY 1000
#endif

static_assert((F_CPU / DSHOT_UPDATE_FREQUENCY) < 65536,"DSHOT_UPDATE_FREQUENCY is too low, chose a higher value" );

#ifndef DSHOT150
#ifndef DSHOT300
#ifndef DSHOT600
//default use dshot 150
//TODO: upgrade to 600 once we know its stable?
#define DSHOT150
#endif
#endif
#endif

//put variables and some functions in namespace to prevent name conflicts, and user from accessing them directly
namespace dshot {

struct dShotData {
	// the bits we precalculate to more quickly write them out
	bool bits[16];

	//precalculated port and bitmasks
	uint8_t pinPort;
	uint8_t pinMaskOn;
	uint8_t pinMaskOff;
};

extern dShotData controllers[DSHOT_CONTROLLER_COUNT];

constexpr long nanosToCycles(const long nanos) {
	return (long) (nanos / ((double) 1000000000 / F_CPU));
}

#ifdef DSHOT150
const uint32_t T1H = nanosToCycles(5000);
const uint32_t T1L = nanosToCycles(6670 - 5000);
const uint32_t T0H = nanosToCycles(2500);
const uint32_t T0L = nanosToCycles(6670 - 2500);
#endif
#ifdef DSHOT300
const uint32_t T1H = nanosToCycles(2500);
const uint32_t T1L = nanosToCycles(3330 - 2500);
const uint32_t T0H = nanosToCycles(1250);
const uint32_t T0L = nanosToCycles(3330 - 1250);
#endif
#ifdef DSHOT600
const uint32_t T1H = nanosToCycles(1250);
const uint32_t T1L = nanosToCycles(1675 - 1250);
const uint32_t T0H = nanosToCycles(625);
const uint32_t T0L = nanosToCycles(1675 - 625);
#endif

constexpr long positiveElseZero(long value) {
	return value > 0 ? value : 0;
}

// creates an arbitrary amount of nops
// usage: nop<5>();
template<long count> __attribute__((always_inline)) inline void nop() {
	nop<positiveElseZero(count - 1)>();
	__asm__ __volatile__("nop\n\t");
}

template<> __attribute__((always_inline)) inline void nop<0>() {
	//nothing so the template recursion ends
}

// converts a pin to the address of a port in an uint8_t (cant return pointer here cause c++ is stupid)
constexpr uint8_t constDigitalPinToPort(const uint8_t pin) {
	return (pin < 8) ? (uint8_t) &PORTD : (pin < 14) ? (uint8_t) &PORTB :
			(pin >= A0 && pin <= A5) ? (uint8_t) &PORTC : 0;
}

// converts pin to offset that is used in switch(On/Off)
constexpr uint8_t constDigitalPinToIndex(const uint8_t pin) {
	return (pin < 8) ? pin : (pin < 14) ? pin - 8 :
			(pin >= A0 && pin <= A5) ? pin - A0 : 0;
}

void sendController(const dShotData data);
void sendData();

const uint8_t calcCrc(const uint16_t value);
const uint16_t createFrame(uint16_t throttle, const bool telemetry = false);

} //namespace end

#endif /* DSHOTDEF_H_ */
