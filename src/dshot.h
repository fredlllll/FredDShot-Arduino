/*
 * dshot.h
 *
 *  Created on: 03.07.2022
 *      Author: Frederik Gelder
 */

#ifndef DSHOT_H_
#define DSHOT_H_
#include "Arduino.h"
#include <stdint.h>

#include "internal/dshotdef.h"

void initDShot();

void setDShotThrottle(const uint8_t controller, const uint16_t throttle,
		const bool telemetry = false);

void setDShotThrottle(const uint16_t throttle, const bool telemetry = false);

void setDShotPin(const uint8_t controller, const uint8_t pin);
void setDShotPin(const uint8_t pin);

#endif /* DSHOT_H_ */
