#include "sources.h"
#include "config.h"
#include "holley_can.h"
#include "inputs.h"
#include "sensors.h"

/* The "pick a value based on source mode" pattern repeats for every gauge,
 * so this macro keeps each accessor down to a single line of intent.
 *
 *   src     : ValueSource — which calibration field decides the policy
 *   canV    : CAN-path value
 *   altV    : non-CAN value (pulse capture or ADC, depends on the gauge)
 *   altOk   : truthy if the local path currently has valid data (e.g.,
 *             pulses are coming in, or sensor is providing data). Used by
 *             LOCAL_PRIMARY to know when to fall back to CAN.
 */
#define PICK(src, canV, altV, altOk)                                       \
    ((src) == SRC_LOCAL_ONLY    ? (altV) :                                 \
     (src) == SRC_CAN_ONLY      ? (Holley_CAN_DataValid() ? (canV) : 0.0f) : \
     (src) == SRC_LOCAL_PRIMARY ? ((altOk) ? (altV) :                      \
                                  (Holley_CAN_DataValid() ? (canV) : 0.0f)) : \
     /* SRC_CAN_PRIMARY (default): CAN if fresh, else local */             \
     (Holley_CAN_DataValid() ? (canV) : (altV)))

float Source_SpeedMph(void)
{
    return PICK(calibration.speedSource,
                gauges.speed, inputs.speedMph,
                inputs.speedMph > 0.0f);
}

float Source_Rpm(void)
{
    return PICK(calibration.tachSource,
                gauges.rpm, inputs.rpm,
                inputs.rpm > 0.0f);
}

float Source_CoolantTempF(void)
{
    /* ADC sensor always has a reading (even if sensor is disconnected we
     * report a sentinel) — treat local as always-available for fallback. */
    return PICK(calibration.coolantTempSource,
                gauges.coolantTemp, sensors.coolantTempF,
                1);
}

float Source_BatteryVoltage(void)
{
    return PICK(calibration.batteryVoltageSource,
                gauges.batteryVoltage, sensors.batteryVoltage,
                1);
}

/* "Have we ever received any speed data?" — covers both CAN-only customers
 * and pulse-only customers without requiring callers to know which path
 * applies. inputs.speedMph stays at 0 if VSS pulse code hasn't seen edges,
 * so a nonzero value is a sufficient signal that the pulse path is alive. */
uint8_t Source_SpeedValid(void)
{
    if (Holley_CAN_DataValid()) return 1;
    return inputs.speedMph > 0.0f;
}

uint8_t Source_RpmValid(void)
{
    if (Holley_CAN_DataValid()) return 1;
    return inputs.rpm > 0.0f;
}
