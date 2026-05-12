#include "sources.h"
#include "config.h"
#include "holley_can.h"
#include "inputs.h"
#include "sensors.h"

/* The "pick a value based on source mode" pattern repeats for every gauge,
 * so this macro keeps each accessor down to a single line of intent.
 *
 *   src   : ValueSource — which calibration field decides the policy
 *   canV  : CAN-path value
 *   altV  : non-CAN value (pulse capture or ADC, depends on the gauge)
 */
#define PICK(src, canV, altV)                                              \
    ((src) == SRC_LOCAL_ONLY                       ? (altV) :              \
     (src) == SRC_CAN_ONLY                         ? (Holley_CAN_DataValid() ? (canV) : 0.0f) : \
     /* SRC_CAN_PRIMARY (default): CAN if fresh, else local */             \
     (Holley_CAN_DataValid() ? (canV) : (altV)))

float Source_SpeedMph(void)
{
    return PICK(calibration.speedSource, gauges.speed, inputs.speedMph);
}

float Source_Rpm(void)
{
    return PICK(calibration.tachSource, gauges.rpm, inputs.rpm);
}

float Source_CoolantTempF(void)
{
    return PICK(calibration.coolantTempSource, gauges.coolantTemp, sensors.coolantTempF);
}

float Source_BatteryVoltage(void)
{
    return PICK(calibration.batteryVoltageSource, gauges.batteryVoltage, sensors.batteryVoltage);
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
