#ifndef CONFIG_H
#define CONFIG_H

#include "main.h"
#include <stdint.h>

/* Where each gauge value should come from. Set per-value so the customer
 * can mix-and-match: e.g., speed from CAN (Sniper 2), tach from a coil-neg
 * pulse, coolant temp from CAN, battery voltage from local ADC.
 *
 * CAN_PRIMARY automatically falls back to the local source when CAN data is
 * stale (>250ms old). The *_ONLY modes never fall back — useful when the
 * customer knows one source is wrong and wants to force the other.
 *
 * Future Bluetooth app exposes these as dropdowns per-gauge. */
typedef enum {
    SRC_CAN_PRIMARY = 0,    /* use CAN if fresh, else local sensor   (default) */
    SRC_LOCAL_ONLY  = 1,    /* always use local sensor; ignore CAN              */
    SRC_CAN_ONLY    = 2,    /* always use CAN; return 0 if stale                */
} ValueSource;

/* Calibration values persisted in I2C EEPROM and adjusted via Bluetooth */
typedef struct {
    /* VSS */
    uint32_t vssPulsesPerMile;      /* T56 default ~8000; calibrate per vehicle */

    /* Oil pressure sender (0.5V–4.5V linear) */
    float    oilSenderMinV;         /* V at 0 PSI   (default 0.5) */
    float    oilSenderMaxV;         /* V at max PSI (default 4.5) */
    float    oilSenderMaxPSI;       /* sender range (default 100) */

    /* Fuel sender (resistive) */
    uint16_t fuelSenderEmptyOhm;    /* resistance at empty (default 10) */
    uint16_t fuelSenderFullOhm;     /* resistance at full  (default 73) */
    uint16_t fuelDividerOhm;        /* known resistor in voltage divider (default 330) */

    /* Coolant sender calibration — curve TBD */

    /* Tachometer */
    uint8_t  tachPulsesPerRev;      /* pulses per crankshaft revolution from signal source */

    /* Per-gauge source selectors. Bluetooth app controls these at runtime. */
    ValueSource speedSource;
    ValueSource tachSource;
    ValueSource coolantTempSource;
    ValueSource batteryVoltageSource;
} CalibrationData;

extern CalibrationData calibration;

void Config_Init(void);     /* load from EEPROM; apply defaults if blank */
void Config_Save(void);     /* write calibration to EEPROM */
void Config_BtUpdate(void); /* poll USART1 for Bluetooth config commands */

#endif /* CONFIG_H */
