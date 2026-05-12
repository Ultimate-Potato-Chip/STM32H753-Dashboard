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

/* Oil pressure sender type. V1 supports the two factory-classic-car common
 * cases on a single OIL terminal. Voltage transducers (0.5-4.5V) are v2
 * (would need a separate hardware input pin with different topology). */
typedef enum {
    OIL_SENDER_SWITCH    = 0,  /* 1-wire low-pressure switch (idiot light) */
    OIL_SENDER_RESISTIVE = 1,  /* variable resistance, 1- or 2-wire (GM/Ford classic) */
} OilSenderType;

/* Coolant temp sender is always thermistor (non-linear).
 * Two ways to calibrate:
 *   - PRESET: pick from built-in curve table for known senders (GM, Ford, etc.)
 *   - CUSTOM: user enters multi-point R/temp pairs via the app
 * Linear interpolation between points in either case. */
typedef enum {
    COOLANT_PROFILE_GM     = 0,  /* GM standard 1980+ */
    COOLANT_PROFILE_FORD   = 1,  /* Ford standard */
    COOLANT_PROFILE_CUSTOM = 2,  /* use coolantCurve[] below */
} CoolantProfile;

/* A point on the resistance/temperature curve. Up to 5 stored.
 * uint32 for resistance because Ford-style thermistors exceed 65kΩ at very low temps. */
typedef struct {
    uint32_t resistance_ohms;
    int16_t  temp_F;
} CoolantCalPoint;

/* Calibration values persisted in I2C EEPROM and adjusted via Bluetooth */
typedef struct {
    /* VSS */
    uint32_t vssPulsesPerMile;      /* T56 default ~8000; calibrate per vehicle */

    /* Oil pressure */
    OilSenderType oilSenderType;    /* default OIL_SENDER_RESISTIVE */
    uint16_t oilSenderZeroOhm;      /* resistance at 0 PSI    (default 0)    */
    uint16_t oilSenderMaxOhm;       /* resistance at maxPSI   (default 90)   */
    uint16_t oilSenderMaxPSI;       /* sender full-scale      (default 100)  */
    uint16_t oilDividerOhm;         /* divider pull-up resistor (default 1000) */
    uint16_t oilSwitchThresholdMV;  /* ADC voltage threshold for switch type (default 1650 = mid-rail) */

    /* Fuel sender (resistive) */
    uint16_t fuelSenderEmptyOhm;    /* resistance at empty (default 10) */
    uint16_t fuelSenderFullOhm;     /* resistance at full  (default 73) */
    uint16_t fuelDividerOhm;        /* known resistor in voltage divider (default 330) */

    /* Coolant sender */
    CoolantProfile  coolantProfile;
    uint16_t        coolantDividerOhm;          /* default 2200 — sized for thermistor range */
    uint8_t         coolantCustomNumPoints;     /* 2-5 if profile = CUSTOM */
    CoolantCalPoint coolantCustomCurve[5];

    /* Battery voltage divider (PC1) — divides 12V down to ADC range */
    float    battDividerRatio;      /* V_battery / V_adc (default 4.4 for 33k/100k divider) */

    /* Dimmer — illumination input voltage at full brightness vs off */
    float    dimmerMinV;            /* V at "off" (default 0.0) */
    float    dimmerMaxV;            /* V at "max bright" (default 12.0 — actual depends on bulb load) */

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
