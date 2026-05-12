#include "holley_can.h"
#include <string.h>

extern FDCAN_HandleTypeDef hfdcan1;

GaugeData gauges = {0};

void Holley_CAN_Init(void)
{
    /* Hardware filter: accept Holley broadcast range (bits 28:11 fixed, bits 10:0 = serial).
     * Mirrors the Holley-specified mask 0xFFFFF800 — pass any ID whose upper bits
     * match the Holley ECU source pattern, regardless of serial number. */
    FDCAN_FilterTypeDef f = {0};
    f.IdType       = FDCAN_EXTENDED_ID;
    f.FilterIndex  = 0;
    f.FilterType   = FDCAN_FILTER_RANGE;
    f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    f.FilterID1    = 0x1E000000U;          /* lowest possible Holley ID (serial=0)  */
    f.FilterID2    = 0x1E1FFFFFU;          /* highest Holley ID we expect + serial  */
    HAL_FDCAN_ConfigFilter(&hfdcan1, &f);

    /* Reject everything that doesn't match a filter */
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,
        FDCAN_REJECT,
        FDCAN_REJECT,
        FDCAN_REJECT_REMOTE,
        FDCAN_REJECT_REMOTE);

    HAL_FDCAN_Start(&hfdcan1);
}

/* Holley sends floats big-endian. Reverse bytes before memcpy on little-endian Cortex-M7.
 * Confirmed against Fan_Controller_1.5_w_GPS.ino canBeToFloat(): { buf[3],buf[2],buf[1],buf[0] } */
static inline float read_float(const uint8_t *d)
{
    uint32_t raw = ((uint32_t)d[0] << 24) |
                   ((uint32_t)d[1] << 16) |
                   ((uint32_t)d[2] <<  8) |
                    (uint32_t)d[3];
    float f;
    memcpy(&f, &raw, 4);
    return f;
}

static void parse_message(uint32_t id, const uint8_t *data)
{
    /* Mask out ECU serial number bits — required per Holley CAN spec */
    id &= CAN_MASK;

    switch (id) {
        case CAN_ID_RPM:
            gauges.rpm = read_float(data);
            gauges.engineRunning = (gauges.rpm > 200);
            break;

        case CAN_ID_CTS:
            gauges.coolantTemp = read_float(data);
            gauges.highCoolantTemp = (gauges.coolantTemp > 220);
            break;

        case CAN_ID_BATTERY:
            gauges.batteryVoltage = read_float(data);
            gauges.lowBattery = (gauges.batteryVoltage < 12.5f && gauges.engineRunning);
            break;

        case CAN_ID_SPEED:
            gauges.speed = read_float(data);
            break;

        case CAN_ID_IAC:
            gauges.iac = read_float(data);
            break;

        case CAN_ID_MAP:
            gauges.map = read_float(data);
            break;

        case CAN_ID_MAT:
            gauges.mat = read_float(data);
            break;

        case CAN_ID_AFR:
            gauges.afr = read_float(data);
            break;

        default:
            break;
    }

    gauges.lastUpdate = HAL_GetTick();
}

void Holley_CAN_Update(void)
{
    FDCAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[8];

    while (HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {
        if (rxHeader.IdType == FDCAN_EXTENDED_ID)
            parse_message(rxHeader.Identifier, rxData);
    }
}

uint8_t Holley_CAN_DataValid(void)
{
    return (HAL_GetTick() - gauges.lastUpdate) < 250U;
}
