#ifndef SOURCES_H
#define SOURCES_H

#include "main.h"
#include <stdint.h>

/* Unified accessors for gauge values that can come from multiple paths
 * (CAN + ADC, or CAN + pulse capture). Each function consults the
 * per-gauge ValueSource in calibration and returns the currently-
 * authoritative value, falling back to the alternative path when the
 * preferred source is stale.
 *
 * Display rendering code should call these instead of reading from
 * `gauges`, `inputs`, or `sensors` directly — keeps source-selection
 * policy in one place. */

float Source_SpeedMph(void);
float Source_Rpm(void);
float Source_CoolantTempF(void);
float Source_BatteryVoltage(void);

/* True when at least one path has a fresh reading for the value.
 * Returns 0 if both CAN is stale AND the local path has no data. */
uint8_t Source_SpeedValid(void);
uint8_t Source_RpmValid(void);

#endif /* SOURCES_H */
