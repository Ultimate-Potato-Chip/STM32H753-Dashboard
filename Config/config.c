#include "config.h"

extern I2C_HandleTypeDef  hi2c1;
extern UART_HandleTypeDef huart1;

static const CalibrationData defaults = {
    .vssPulsesPerMile     = 8000,
    .oilSenderMinV        = 0.5f,
    .oilSenderMaxV        = 4.5f,
    .oilSenderMaxPSI      = 100.0f,
    .fuelSenderEmptyOhm   = 10,
    .fuelSenderFullOhm    = 73,
    .fuelDividerOhm       = 330,
    .tachPulsesPerRev     = 1,

    /* CAN preferred for everything by default — customers with a Sniper 2
     * get full functionality automatically, customers without CAN fall back
     * to local sensors transparently. App can override per-gauge. */
    .speedSource          = SRC_CAN_PRIMARY,
    .tachSource           = SRC_CAN_PRIMARY,
    .coolantTempSource    = SRC_CAN_PRIMARY,
    .batteryVoltageSource = SRC_CAN_PRIMARY,
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
