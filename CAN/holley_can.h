#ifndef HOLLEY_CAN_H
#define HOLLEY_CAN_H

#include "main.h"
#include <stdint.h>

/* Holley EFI CAN protocol:
 *   - Extended 29-bit IDs, 1 Mbit/s
 *   - Bits 10:0 of the CAN ID are the ECU serial number — mask with 0xFFFFF800 before comparing
 *   - Data payload: bytes 0-3 = IEEE 754 float (value), bytes 4-7 = u32 status
 *
 * IDs below are the masked values (source serial = 0), matching the DBC after stripping bit 31. */
#define CAN_MASK          0xFFFFF800U

#define CAN_ID_RPM        0x1E005000U   /* RPM                  float, RPM     */
#define CAN_ID_AFR        0x1E019000U   /* AirFuel_Ratio        float, A/F     */
#define CAN_ID_TARGET_AFR 0x1E015000U   /* Target_AFR           float, A/F     */
#define CAN_ID_IAC        0x1E059000U   /* IAC_Position         float, %       */
#define CAN_ID_MAP        0x1E05D000U   /* MAP                  float, kPa     */
#define CAN_ID_MAT        0x1E065000U   /* MAT                  float, °F      */
#define CAN_ID_CTS        0x1E069000U   /* CTS (coolant temp)   float, °F      */
#define CAN_ID_BATTERY    0x1E06D000U   /* Battery              float, V       */
#define CAN_ID_SPEED      0x1E1C1000U   /* Speed                float, MPH     */

typedef struct {
    float    rpm;
    float    coolantTemp;       /* °F — CTS message */
    float    batteryVoltage;    /* V  */
    float    speed;             /* MPH */
    float    oilPressure;       /* PSI (ADC, not CAN) */
    float    fuelLevel;         /* %   (ADC, not CAN) */
    float    iac;
    float    map;               /* kPa */
    float    mat;               /* °F  */
    float    afr;
    uint8_t  engineRunning;
    uint8_t  lowOilPressure;
    uint8_t  lowBattery;
    uint8_t  highCoolantTemp;
    uint32_t lastUpdate;        /* HAL_GetTick() at last CAN message */
} GaugeData;

extern GaugeData gauges;

void    Holley_CAN_Init(void);
void    Holley_CAN_Update(void);   /* call each main-loop iteration */
uint8_t Holley_CAN_DataValid(void);

#endif /* HOLLEY_CAN_H */
