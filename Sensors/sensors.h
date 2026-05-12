#ifndef SENSORS_H
#define SENSORS_H

#include "main.h"
#include <stdint.h>

typedef struct {
    float oilPressure;      /* PSI  — ADC PA1, 0-5V sender */
    float coolantTempF;     /* °F   — ADC PA2 */
    float batteryVoltage;   /* V    — ADC PC1 */
    float fuelLevel;        /* %    — ADC PC5, resistive sender */
    float dimmerVoltage;    /* V    — ADC PF12: 0V=lights off, >0V=rheostat */
    uint8_t lowOilPressure;
    uint8_t highCoolantTemp;
    uint8_t lowBattery;
} SensorData;

extern SensorData sensors;

void Sensors_Init(void);
void Sensors_Update(void);   /* call each main-loop iteration */

#endif /* SENSORS_H */
