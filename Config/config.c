#include "config.h"
#include <string.h>

extern I2C_HandleTypeDef  hi2c1;
extern UART_HandleTypeDef huart1;

/* -------------------------------------------------------------------------
 * Defaults — used when EEPROM is blank, corrupted, or unreachable.
 * -------------------------------------------------------------------------*/
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

    /* Source defaults: VSS prefers local pulse (faster update than CAN); others
     * prefer CAN (Sniper-style ECUs publish vitals authoritatively). */
    .speedSource             = SRC_LOCAL_PRIMARY,
    .tachSource              = SRC_CAN_PRIMARY,
    .coolantTempSource       = SRC_CAN_PRIMARY,
    .batteryVoltageSource    = SRC_CAN_PRIMARY,
};

CalibrationData calibration;

/* Cached "last successfully written" copy — used by Config_Save to avoid
 * touching the EEPROM unless something actually changed. EEPROMs are rated
 * ~1M write cycles; saving on every boot or every Bluetooth poll would burn
 * through that surprisingly fast in a long-lived product. */
static CalibrationData lastSaved;
static uint8_t          haveLastSaved = 0;

/* -------------------------------------------------------------------------
 * EEPROM protocol
 *
 * Targeting 24LC256-class chips: 16-bit internal address, 64-byte page
 * boundary, ~5ms write cycle, I2C device address 0xA0 (8-bit).
 *
 * Storage layout starting at address 0x0000:
 *   magic       (uint32) — 'F100' signature so we can detect blank/foreign data
 *   formatVer   (uint16) — schema version, lets us migrate fields later
 *   structSize  (uint16) — sizeof(CalibrationData); rejects size mismatch
 *   data        (CalibrationData)
 *   crc16       (uint16) — CRC-CCITT over the data field
 *
 * On read: validate magic + version + size + CRC. Any mismatch → defaults.
 * On write: only if calibration differs from lastSaved cache.
 * -------------------------------------------------------------------------*/
#define EEPROM_I2C_ADDR     0xA0u
#define EEPROM_BASE_ADDR    0x0000u
#define EEPROM_PAGE_SIZE    64u
#define EEPROM_WRITE_MS     5u          /* HAL_Delay after each page write */
#define EEPROM_MAGIC        0x46313030u /* 'F100' */
#define EEPROM_FORMAT_VER   1u

typedef struct __attribute__((packed)) {
    uint32_t        magic;
    uint16_t        formatVer;
    uint16_t        structSize;
    CalibrationData data;
    uint16_t        crc16;
} EepromBlob;

/* CRC-16/CCITT-FALSE — standard polynomial 0x1021, init 0xFFFF. */
static uint16_t crc16_ccitt(const uint8_t *buf, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)buf[i] << 8;
        for (int b = 0; b < 8; b++) {
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

static HAL_StatusTypeDef eeprom_read(uint16_t addr, uint8_t *buf, uint16_t len)
{
    return HAL_I2C_Mem_Read(&hi2c1, EEPROM_I2C_ADDR,
                            addr, I2C_MEMADD_SIZE_16BIT,
                            buf, len, 100);
}

/* Page-aware write: splits at 64-byte boundaries so a write that crosses
 * a page wrap-around doesn't silently overwrite the wrong cells. */
static HAL_StatusTypeDef eeprom_write(uint16_t addr, const uint8_t *buf, uint16_t len)
{
    while (len > 0) {
        uint16_t pageOffset = addr % EEPROM_PAGE_SIZE;
        uint16_t chunk      = EEPROM_PAGE_SIZE - pageOffset;
        if (chunk > len) chunk = len;

        HAL_StatusTypeDef st = HAL_I2C_Mem_Write(&hi2c1, EEPROM_I2C_ADDR,
                                                  addr, I2C_MEMADD_SIZE_16BIT,
                                                  (uint8_t *)buf, chunk, 100);
        if (st != HAL_OK) return st;
        HAL_Delay(EEPROM_WRITE_MS);    /* let the chip's internal cycle finish */

        addr += chunk;
        buf  += chunk;
        len  -= chunk;
    }
    return HAL_OK;
}

/* -------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------*/

void Config_Init(void)
{
    EepromBlob        blob = {0};
    HAL_StatusTypeDef st   = eeprom_read(EEPROM_BASE_ADDR, (uint8_t *)&blob, sizeof(blob));

    /* Any of: I2C error, wrong magic, wrong format version, wrong struct size,
     * or CRC mismatch → blob is untrustworthy. Fall back to defaults and
     * leave the EEPROM untouched (no first-boot write — keeps cycle count low
     * and lets us start with a missing/unwired EEPROM during development). */
    if (st          != HAL_OK ||
        blob.magic  != EEPROM_MAGIC ||
        blob.formatVer != EEPROM_FORMAT_VER ||
        blob.structSize != sizeof(CalibrationData))
    {
        calibration = defaults;
        haveLastSaved = 0;
        return;
    }

    uint16_t computed = crc16_ccitt((const uint8_t *)&blob.data, sizeof(blob.data));
    if (computed != blob.crc16) {
        calibration = defaults;
        haveLastSaved = 0;
        return;
    }

    /* Valid blob — adopt it and remember it as the last-saved snapshot. */
    calibration   = blob.data;
    lastSaved     = blob.data;
    haveLastSaved = 1;
}

void Config_Save(void)
{
    /* Skip the write entirely if nothing changed since the last successful
     * persist. This is the main wear-leveling mechanism — callers can invoke
     * Config_Save() whenever convenient without burning EEPROM cycles. */
    if (haveLastSaved && memcmp(&calibration, &lastSaved, sizeof(calibration)) == 0) {
        return;
    }

    EepromBlob blob;
    blob.magic      = EEPROM_MAGIC;
    blob.formatVer  = EEPROM_FORMAT_VER;
    blob.structSize = sizeof(CalibrationData);
    blob.data       = calibration;
    blob.crc16      = crc16_ccitt((const uint8_t *)&blob.data, sizeof(blob.data));

    if (eeprom_write(EEPROM_BASE_ADDR, (const uint8_t *)&blob, sizeof(blob)) == HAL_OK) {
        lastSaved     = calibration;
        haveLastSaved = 1;
    }
    /* On failure: leave lastSaved stale so the next Config_Save retries.
     * No fault propagated upward — gauge cluster should keep working even
     * if EEPROM is dead. */
}

void Config_BtUpdate(void)
{
    /* TODO: non-blocking read from huart1.
     * Accept a simple line-based protocol, e.g.:
     *   "SET VSS_PPM 8192\n"
     *   "SET FUEL_EMPTY 12\n"
     *   "GET ALL\n"
     *
     * On any field write, mutate `calibration` then call Config_Save() —
     * the save is a no-op unless something actually changed. */
}
