#include "sensors.h"
#include "config.h"

extern ADC_HandleTypeDef hadc1;

SensorData sensors = {0};

/* PC5 = ADC1_INP8 — verify against .ioc if readings look wrong */
#define ADC_CH_FUEL     ADC_CHANNEL_8

/* Single-channel blocking read. Reuse for each sensor — fine for a 1 kHz main loop. */
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

static void read_fuel(void)
{
    uint32_t raw     = adc_read(ADC_CH_FUEL);
    float    voltage = (raw / 65535.0f) * 3.3f;

    /* Avoid divide-by-zero at rail voltage */
    if (voltage >= 3.29f) { sensors.fuelLevel = 0.0f; return; }

    /* Voltage divider: 3.3V —[fuelDividerOhm]— PC5 —[sender]— GND
     * Solve for sender resistance: R = R_div * V / (3.3 - V) */
    float r = (calibration.fuelDividerOhm * voltage) / (3.3f - voltage);

    /* Map resistance to 0-100%: Ford sender 73Ω=full, 10Ω=empty (calibration defaults) */
    float pct = (r - calibration.fuelSenderEmptyOhm) /
                (float)(calibration.fuelSenderFullOhm - calibration.fuelSenderEmptyOhm) * 100.0f;

    if (pct < 0.0f)   pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    sensors.fuelLevel = pct;
}

void Sensors_Init(void)
{
    /* Configure ADC pins not set up by CubeMX as analog inputs */
    GPIO_InitTypeDef g = {0};
    g.Mode = GPIO_MODE_ANALOG;
    g.Pull = GPIO_NOPULL;

    g.Pin = GPIO_PIN_5;   HAL_GPIO_Init(GPIOC, &g);   /* PC5  — fuel level    */
    /* TODO: add PA1, PA2, PC1, PF12 here as remaining channels are implemented */

    HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
}

void Sensors_Update(void)
{
    read_fuel();
    /* TODO: read_oil(), read_coolant(), read_battery(), read_dimmer() */
}
