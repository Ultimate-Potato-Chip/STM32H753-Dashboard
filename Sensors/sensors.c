#include "sensors.h"
#include "config.h"

extern ADC_HandleTypeDef hadc1;

SensorData sensors = {0};

/* ADC channel assignments on ADC1 for STM32H753ZI (confirmed against datasheet):
 *   PA1 = INP17  (oil pressure)
 *   PA2 = INP14  (coolant temp)
 *   PC1 = INP11  (battery voltage)
 *   PC5 = INP8   (fuel level)
 *   PF12 = INP6  (dimmer)
 * If readings look wrong, verify the .ioc and re-check the channel numbers. */
#define ADC_CH_OIL      ADC_CHANNEL_17
#define ADC_CH_COOLANT  ADC_CHANNEL_14
#define ADC_CH_BATTERY  ADC_CHANNEL_11
#define ADC_CH_FUEL     ADC_CHANNEL_8
#define ADC_CH_DIMMER   ADC_CHANNEL_6

/* Sender excitation rail. Currently 3.3V — TODO migrate to 12V (per design doc)
 * for noise immunity in production hardware. When that happens, change here
 * and re-size the divider resistors in calibration defaults. */
#define SENSOR_EXCITATION_V     3.3f

/* Low-pressure warning threshold for resistive oil senders (PSI). */
#define OIL_LOW_PSI_THRESHOLD   5.0f

/* High-coolant alarm threshold (°F). */
#define COOLANT_HIGH_F          220.0f

/* Low-battery alarm threshold (V) when engine running. */
#define BATTERY_LOW_V           12.5f

/* -------------------------------------------------------------------------
 * Coolant temp preset curves (resistance/temp pairs, NTC thermistor type)
 * Linear interpolation between adjacent points. Data points sourced from
 * published manufacturer curves — verify against actual sender before
 * shipping each preset.
 * -------------------------------------------------------------------------*/
typedef struct {
    uint8_t numPoints;
    CoolantCalPoint points[5];
} CoolantPresetCurve;

static const CoolantPresetCurve coolantPresetGM = {
    .numPoints = 5,
    .points = {
        { 13500,  32 },  /* 32°F   = ~13.5kΩ */
        {  3520, 100 },
        {  1490, 160 },
        {   470, 212 },
        {   175, 280 },
    }
};

static const CoolantPresetCurve coolantPresetFord = {
    .numPoints = 5,
    .points = {
        { 95000,  32 },  /* Ford senders run higher resistance than GM */
        { 22000, 100 },
        {  4900, 160 },
        {  1550, 212 },
        {   500, 280 },
    }
};

/* -------------------------------------------------------------------------
 * ADC helpers
 * -------------------------------------------------------------------------*/
static uint32_t adc_read(uint32_t channel)
{
    ADC_ChannelConfTypeDef cfg = {0};
    cfg.Channel      = channel;
    cfg.Rank         = ADC_REGULAR_RANK_1;
    cfg.SamplingTime = ADC_SAMPLETIME_387CYCLES_5;  /* long sample for high-Z resistive senders */
    cfg.SingleDiff   = ADC_SINGLE_ENDED;
    cfg.OffsetNumber = ADC_OFFSET_NONE;
    HAL_ADC_ConfigChannel(&hadc1, &cfg);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint32_t val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return val;
}

/* Convert raw 16-bit ADC reading to voltage at the pin. */
static inline float adc_to_v(uint32_t raw)
{
    return ((float)raw / 65535.0f) * SENSOR_EXCITATION_V;
}

/* Solve voltage divider for the sender resistance.
 *   V_excite ── R_known ── V_pin ── R_sender ── GND
 *   R_sender = R_known × V_pin / (V_excite - V_pin)
 * Returns 0 if voltage is at the rail (would be divide-by-zero). */
static float sender_resistance(float v_pin, float r_known)
{
    float vRail = SENSOR_EXCITATION_V - 0.01f;
    if (v_pin >= vRail) return 0.0f;
    return (r_known * v_pin) / (SENSOR_EXCITATION_V - v_pin);
}

/* Linear interpolation across a coolant preset curve.
 * Points must be sorted by resistance descending (high R = cold, low R = hot). */
static float interpolate_curve(const CoolantPresetCurve *curve, float r)
{
    if (curve->numPoints == 0) return -999.0f;

    /* Off the high-R end → colder than highest cal point */
    if (r >= curve->points[0].resistance_ohms) return curve->points[0].temp_F;

    /* Off the low-R end → hotter than lowest cal point */
    if (r <= curve->points[curve->numPoints - 1].resistance_ohms) {
        return curve->points[curve->numPoints - 1].temp_F;
    }

    for (uint8_t i = 0; i < curve->numPoints - 1; i++) {
        uint32_t rHi = curve->points[i].resistance_ohms;       /* higher R, lower T */
        uint32_t rLo = curve->points[i + 1].resistance_ohms;   /* lower R, higher T */
        if (r <= rHi && r >= rLo) {
            int16_t tLo = curve->points[i].temp_F;
            int16_t tHi = curve->points[i + 1].temp_F;
            float fraction = (float)(rHi - r) / (float)(rHi - rLo);
            return tLo + fraction * (tHi - tLo);
        }
    }
    return -999.0f;  /* shouldn't reach */
}

/* -------------------------------------------------------------------------
 * Sender readings
 * -------------------------------------------------------------------------*/

static void read_fuel(void)
{
    float v = adc_to_v(adc_read(ADC_CH_FUEL));
    if (v >= SENSOR_EXCITATION_V - 0.01f) { sensors.fuelLevel = 0.0f; return; }

    float r = sender_resistance(v, calibration.fuelDividerOhm);
    float pct = (r - calibration.fuelSenderEmptyOhm) /
                (float)(calibration.fuelSenderFullOhm - calibration.fuelSenderEmptyOhm) * 100.0f;

    if (pct < 0.0f)   pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    sensors.fuelLevel = pct;
}

static void read_oil(void)
{
    float v = adc_to_v(adc_read(ADC_CH_OIL));

    if (calibration.oilSenderType == OIL_SENDER_SWITCH) {
        /* Pressure switch: low voltage = switch closed to ground = LOW pressure */
        float thresholdV = (float)calibration.oilSwitchThresholdMV / 1000.0f;
        sensors.lowOilPressure = (v < thresholdV) ? 1 : 0;
        sensors.oilPressure = 0.0f;  /* no PSI value available in switch mode */
        return;
    }

    /* OIL_SENDER_RESISTIVE: voltage divider → resistance → PSI */
    if (v >= SENSOR_EXCITATION_V - 0.01f) {
        sensors.oilPressure = 0.0f;
        sensors.lowOilPressure = 1;
        return;
    }

    float r = sender_resistance(v, calibration.oilDividerOhm);
    int16_t rangeOhm = (int16_t)calibration.oilSenderMaxOhm - (int16_t)calibration.oilSenderZeroOhm;
    float psi = 0.0f;
    if (rangeOhm != 0) {
        psi = ((float)r - (float)calibration.oilSenderZeroOhm) / (float)rangeOhm
              * (float)calibration.oilSenderMaxPSI;
    }
    if (psi < 0.0f) psi = 0.0f;
    sensors.oilPressure = psi;
    sensors.lowOilPressure = (psi < OIL_LOW_PSI_THRESHOLD) ? 1 : 0;
}

static void read_coolant(void)
{
    float v = adc_to_v(adc_read(ADC_CH_COOLANT));
    if (v >= SENSOR_EXCITATION_V - 0.01f) {
        sensors.coolantTempF = -40.0f;  /* sensor disconnected or open-circuit */
        sensors.highCoolantTemp = 0;
        return;
    }

    float r = sender_resistance(v, calibration.coolantDividerOhm);

    const CoolantPresetCurve *curve = NULL;
    switch (calibration.coolantProfile) {
        case COOLANT_PROFILE_GM:     curve = &coolantPresetGM;     break;
        case COOLANT_PROFILE_FORD:   curve = &coolantPresetFord;   break;
        case COOLANT_PROFILE_CUSTOM:
            /* Build a temporary curve from calibration data on the stack */
        {
            CoolantPresetCurve custom;
            custom.numPoints = calibration.coolantCustomNumPoints;
            for (uint8_t i = 0; i < custom.numPoints && i < 5; i++) {
                custom.points[i] = calibration.coolantCustomCurve[i];
            }
            sensors.coolantTempF = interpolate_curve(&custom, r);
            sensors.highCoolantTemp = (sensors.coolantTempF > COOLANT_HIGH_F) ? 1 : 0;
            return;
        }
    }

    if (curve) {
        sensors.coolantTempF = interpolate_curve(curve, r);
        sensors.highCoolantTemp = (sensors.coolantTempF > COOLANT_HIGH_F) ? 1 : 0;
    }
}

static void read_battery(void)
{
    float v_adc = adc_to_v(adc_read(ADC_CH_BATTERY));
    sensors.batteryVoltage = v_adc * calibration.battDividerRatio;
    sensors.lowBattery = (sensors.batteryVoltage < BATTERY_LOW_V) ? 1 : 0;
}

static void read_dimmer(void)
{
    /* Same divider topology as battery — illumination wire goes through a
     * resistor divider to bring 0-12V down to ADC range. We report the
     * actual rail voltage; brightness logic elsewhere maps to display PWM. */
    float v_adc = adc_to_v(adc_read(ADC_CH_DIMMER));
    sensors.dimmerVoltage = v_adc * calibration.battDividerRatio;  /* same divider ratio */
}

/* -------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------*/

void Sensors_Init(void)
{
    GPIO_InitTypeDef g = {0};
    g.Mode = GPIO_MODE_ANALOG;
    g.Pull = GPIO_NOPULL;

    g.Pin = GPIO_PIN_1;  HAL_GPIO_Init(GPIOA, &g);  /* PA1  — oil pressure  */
    g.Pin = GPIO_PIN_2;  HAL_GPIO_Init(GPIOA, &g);  /* PA2  — coolant temp  */
    g.Pin = GPIO_PIN_1;  HAL_GPIO_Init(GPIOC, &g);  /* PC1  — battery       */
    g.Pin = GPIO_PIN_5;  HAL_GPIO_Init(GPIOC, &g);  /* PC5  — fuel level    */
    g.Pin = GPIO_PIN_12; HAL_GPIO_Init(GPIOF, &g);  /* PF12 — dimmer        */

    HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
}

void Sensors_Update(void)
{
    read_fuel();
    read_oil();
    read_coolant();
    read_battery();
    read_dimmer();
}
