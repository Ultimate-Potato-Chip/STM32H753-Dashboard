#include "config.h"

extern I2C_HandleTypeDef  hi2c1;
extern UART_HandleTypeDef huart1;

static const CalibrationData defaults = {
    .vssPulsesPerMile        = 8000,
    .vssPulsesToAverage      = 5,        /* ~1/4 of a 17-tooth T56 — Holley convention */

    /* Oil pressure: GM 0-90Ω resistive sender, 100 PSI range */
    .oilSenderType           = OIL_SENDER_RESISTIVE,
    .oilSenderZeroOhm        = 0,
    .oilSenderMaxOhm         = 90,
    .oilSenderMaxPSI         = 100,
    .oilDividerOhm           = 1000,
    .oilSwitchThresholdMV    = 1650,    /* mid-rail */

    /* Fuel: Ford-style 73Ω full / 10Ω empty, 330Ω divider */
    .fuelSenderEmptyOhm      = 10,
    .fuelSenderFullOhm       = 73,
    .fuelDividerOhm          = 330,

    /* Coolant: GM thermistor preset */
    .coolantProfile          = COOLANT_PROFILE_GM,
    .coolantDividerOhm       = 2200,
    .coolantCustomNumPoints  = 0,

    /* Battery: 100k/33k divider gives ratio 4.03 (V_batt = V_adc * 4.03 worst case) */
    .battDividerRatio        = 4.03f,

    /* Dimmer */
    .dimmerMinV              = 0.0f,
    .dimmerMaxV              = 12.0f,

    .tachPulsesPerRev        = 1,

    /* Defaults:
     *   Speed: LOCAL_PRIMARY — direct pulse capture updates faster than CAN
     *          broadcast rate at highway speeds, and is the "natural" reading
     *          when a physical VSS is wired. Falls back to CAN if no pulses.
     *   Others: CAN_PRIMARY — Sniper-style ECUs publish these at adequate
     *          rates with built-in smoothing, and the CAN value is generally
     *          more authoritative for engine vitals. */
    .speedSource             = SRC_LOCAL_PRIMARY,
    .tachSource              = SRC_CAN_PRIMARY,
    .coolantTempSource       = SRC_CAN_PRIMARY,
    .batteryVoltageSource    = SRC_CAN_PRIMARY,
};

CalibrationData calibration;

void Config_Init(void)
{
    /* TODO: read CalibrationData from EEPROM at I2C address 0xA0.
     * If blank (0xFF bytes) or CRC mismatch, apply defaults. */
    calibration = defaults;
}

void Config_Save(void)
{
    /* TODO: write calibration struct to EEPROM with CRC */
}

void Config_BtUpdate(void)
{
    /* TODO: non-blocking read from huart1.
     * Accept a simple line-based protocol, e.g.:
     *   "SET VSS_PPM 8192\n"
     *   "SET FUEL_EMPTY 12\n"
     *   "GET ALL\n"
     * Update calibration fields and call Config_Save() on change. */
}
