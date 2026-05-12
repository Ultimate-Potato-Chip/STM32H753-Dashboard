#include "st77916.h"

/* Manufacturer's init sequence — do not modify */
typedef struct {
    uint8_t reg;
    uint8_t data[36];
    uint8_t len;
} lcd_init_t;

static const lcd_init_t LCD_InitTable[] = {
    {0xF0,{0x28},1}, {0xF2,{0x28},1}, {0x73,{0xF0},1}, {0x7C,{0xD1},1},
    {0x83,{0xE0},1}, {0x84,{0x61},1}, {0xF2,{0x82},1}, {0xF0,{0x00},1},
    {0xF0,{0x01},1}, {0xF1,{0x01},1},
    {0xB0,{0x69},1}, {0xB1,{0x4A},1}, {0xB2,{0x2F},1}, {0xB3,{0x01},1},
    {0xB4,{0x69},1}, {0xB5,{0x45},1}, {0xB6,{0xAB},1}, {0xB7,{0x41},1},
    {0xB8,{0x86},1}, {0xB9,{0x15},1}, {0xBA,{0x00},1}, {0xBB,{0x08},1},
    {0xBC,{0x08},1}, {0xBD,{0x00},1}, {0xBE,{0x00},1}, {0xBF,{0x07},1},
    {0xC0,{0x80},1}, {0xC1,{0x10},1}, {0xC2,{0x37},1}, {0xC3,{0x80},1},
    {0xC4,{0x10},1}, {0xC5,{0x37},1}, {0xC6,{0xA9},1}, {0xC7,{0x41},1},
    {0xC8,{0x01},1}, {0xC9,{0xA9},1}, {0xCA,{0x41},1}, {0xCB,{0x01},1},
    {0xCC,{0x7F},1}, {0xCD,{0x7F},1}, {0xCE,{0xFF},1},
    {0xD0,{0x91},1}, {0xD1,{0x68},1}, {0xD2,{0x68},1},
    {0xF5,{0x00,0xA5},2}, {0xF1,{0x10},1}, {0xF0,{0x00},1}, {0xF0,{0x02},1},
    {0xE0,{0xF0,0x10,0x18,0x0D,0x0C,0x38,0x3E,0x44,0x51,0x39,0x15,0x15,0x30,0x34},14},
    {0xE1,{0xF0,0x0F,0x17,0x0D,0x0B,0x07,0x3E,0x33,0x51,0x39,0x15,0x15,0x30,0x34},14},
    {0xF0,{0x10},1}, {0xF3,{0x10},1},
    {0xE0,{0x08},1}, {0xE1,{0x00},1}, {0xE2,{0x00},1}, {0xE3,{0x00},1},
    {0xE4,{0xE0},1}, {0xE5,{0x06},1}, {0xE6,{0x21},1}, {0xE7,{0x03},1},
    {0xE8,{0x05},1}, {0xE9,{0x02},1}, {0xEA,{0xE9},1}, {0xEB,{0x00},1},
    {0xEC,{0x00},1}, {0xED,{0x14},1}, {0xEE,{0xFF},1}, {0xEF,{0x00},1},
    {0xF8,{0xFF},1}, {0xF9,{0x00},1}, {0xFA,{0x00},1}, {0xFB,{0x30},1},
    {0xFC,{0x00},1}, {0xFD,{0x00},1}, {0xFE,{0x00},1}, {0xFF,{0x00},1},
    {0x60,{0x40},1}, {0x61,{0x05},1}, {0x62,{0x00},1}, {0x63,{0x42},1},
    {0x64,{0xDA},1}, {0x65,{0x00},1}, {0x66,{0x00},1}, {0x67,{0x00},1},
    {0x68,{0x00},1}, {0x69,{0x00},1}, {0x6A,{0x00},1}, {0x6B,{0x00},1},
    {0x70,{0x40},1}, {0x71,{0x04},1}, {0x72,{0x00},1}, {0x73,{0x42},1},
    {0x74,{0xD9},1}, {0x75,{0x00},1}, {0x76,{0x00},1}, {0x77,{0x00},1},
    {0x78,{0x00},1}, {0x79,{0x00},1}, {0x7A,{0x00},1}, {0x7B,{0x00},1},
    {0x80,{0x48},1}, {0x81,{0x00},1}, {0x82,{0x07},1}, {0x83,{0x02},1},
    {0x84,{0xD7},1}, {0x85,{0x04},1}, {0x86,{0x00},1}, {0x87,{0x00},1},
    {0x88,{0x48},1}, {0x89,{0x00},1}, {0x8A,{0x09},1}, {0x8B,{0x02},1},
    {0x8C,{0xD9},1}, {0x8D,{0x04},1}, {0x8E,{0x00},1}, {0x8F,{0x00},1},
    {0x90,{0x48},1}, {0x91,{0x00},1}, {0x92,{0x0B},1}, {0x93,{0x02},1},
    {0x94,{0xDB},1}, {0x95,{0x04},1}, {0x96,{0x00},1}, {0x97,{0x00},1},
    {0x98,{0x48},1}, {0x99,{0x00},1}, {0x9A,{0x0D},1}, {0x9B,{0x02},1},
    {0x9C,{0xDD},1}, {0x9D,{0x04},1}, {0x9E,{0x00},1}, {0x9F,{0x00},1},
    {0xA0,{0x48},1}, {0xA1,{0x00},1}, {0xA2,{0x06},1}, {0xA3,{0x02},1},
    {0xA4,{0xD6},1}, {0xA5,{0x04},1}, {0xA6,{0x00},1}, {0xA7,{0x00},1},
    {0xA8,{0x48},1}, {0xA9,{0x00},1}, {0xAA,{0x08},1}, {0xAB,{0x02},1},
    {0xAC,{0xD8},1}, {0xAD,{0x04},1}, {0xAE,{0x00},1}, {0xAF,{0x00},1},
    {0xB0,{0x48},1}, {0xB1,{0x00},1}, {0xB2,{0x0A},1}, {0xB3,{0x02},1},
    {0xB4,{0xDA},1}, {0xB5,{0x04},1}, {0xB6,{0x00},1}, {0xB7,{0x00},1},
    {0xB8,{0x48},1}, {0xB9,{0x00},1}, {0xBA,{0x0C},1}, {0xBB,{0x02},1},
    {0xBC,{0xDC},1}, {0xBD,{0x04},1}, {0xBE,{0x00},1}, {0xBF,{0x00},1},
    {0xC0,{0x10},1}, {0xC1,{0x47},1}, {0xC2,{0x56},1}, {0xC3,{0x65},1},
    {0xC4,{0x74},1}, {0xC5,{0x88},1}, {0xC6,{0x99},1}, {0xC7,{0x01},1},
    {0xC8,{0xBB},1}, {0xC9,{0xAA},1},
    {0xD0,{0x10},1}, {0xD1,{0x47},1}, {0xD2,{0x56},1}, {0xD3,{0x65},1},
    {0xD4,{0x74},1}, {0xD5,{0x88},1}, {0xD6,{0x99},1}, {0xD7,{0x01},1},
    {0xD8,{0xBB},1}, {0xD9,{0xAA},1},
    {0xF3,{0x01},1}, {0xF0,{0x00},1},
    {0x36,{0x00},1},   /* MADCTL - portrait, RGB */
    {0x3A,{0x05},1},   /* COLMOD - RGB565 */
    {0x35,{0x00},1},   /* Tearing effect on */
    {0x21,{0x00},0},   /* Display inversion on */
    {0x11,{0x00},0},   /* Sleep out */
    {0xFF,{120},1},    /* Delay 120ms */
    {0x29,{0x00},0},   /* Display on */
    {0x2C,{0x00},0},   /* Memory write */
};

/* CS pin lookup table */
static const struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} cs_pins[] = {
    {ST77916_CS1_GPIO_Port, ST77916_CS1_Pin},
    {ST77916_CS2_GPIO_Port, ST77916_CS2_Pin},
    {ST77916_CS3_GPIO_Port, ST77916_CS3_Pin},
    {ST77916_CS4_GPIO_Port, ST77916_CS4_Pin},
};

extern QSPI_HandleTypeDef hqspi;

static void qspi_send_cmd(uint8_t instruction, uint32_t addr, uint32_t addr_mode,
                           uint32_t data_mode, const uint8_t *data, uint32_t len)
{
    HAL_QSPI_Abort(&hqspi);

    QSPI_CommandTypeDef s = {0};
    s.Instruction        = instruction;
    s.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    s.Address            = (uint32_t)addr << 8;  /* register in bits 15-8, matches ESP32 */
    s.AddressSize        = QSPI_ADDRESS_24_BITS;
    s.AddressMode        = addr_mode;
    s.DataMode           = (len > 0) ? data_mode : QSPI_DATA_NONE;
    s.NbData             = len;
    s.DummyCycles        = 0;
    s.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;
    s.DdrMode            = QSPI_DDR_MODE_DISABLE;
    s.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;

    HAL_QSPI_Command(&hqspi, &s, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    if (len > 0 && data != NULL)
        HAL_QSPI_Transmit(&hqspi, (uint8_t *)data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
}

void ST77916_Select(ST77916_Display_t display)
{
    if (display == ST77916_DISPLAY_ALL) {
        for (int i = 0; i < 4; i++)
            HAL_GPIO_WritePin(cs_pins[i].port, cs_pins[i].pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(cs_pins[display].port, cs_pins[display].pin, GPIO_PIN_RESET);
    }
}

void ST77916_Deselect(ST77916_Display_t display)
{
    if (display == ST77916_DISPLAY_ALL) {
        for (int i = 0; i < 4; i++)
            HAL_GPIO_WritePin(cs_pins[i].port, cs_pins[i].pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(cs_pins[display].port, cs_pins[display].pin, GPIO_PIN_SET);
    }
}

static void st77916_send_init_sequence(ST77916_Display_t display)
{
    for (uint16_t i = 0; i < sizeof(LCD_InitTable) / sizeof(lcd_init_t); i++) {
        const lcd_init_t *cmd = &LCD_InitTable[i];
        if (cmd->reg == 0xFF) {
            HAL_Delay(cmd->data[0]);
        } else {
            ST77916_Select(display);
            qspi_send_cmd(LCD_CMD_SINGLE, cmd->reg, QSPI_ADDRESS_1_LINE,
                          QSPI_DATA_1_LINE, cmd->data, cmd->len);
            ST77916_Deselect(display);
        }
    }
}

void ST77916_Init(ST77916_Display_t display)
{
    /* Hardware reset — shared across all displays, do once before InitAll */
    HAL_GPIO_WritePin(ST77916_RST_GPIO_Port, ST77916_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(ST77916_RST_GPIO_Port, ST77916_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(ST77916_RST_GPIO_Port, ST77916_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(120);
    st77916_send_init_sequence(display);
}

void ST77916_InitAll(void)
{
    /* Reset once — RST is shared across all displays */
    HAL_GPIO_WritePin(ST77916_RST_GPIO_Port, ST77916_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(ST77916_RST_GPIO_Port, ST77916_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(ST77916_RST_GPIO_Port, ST77916_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(120);
    for (int i = 0; i < 4; i++)
        st77916_send_init_sequence((ST77916_Display_t)i);
}

void ST77916_SetWindow(ST77916_Display_t display, uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
    uint8_t x_pos[4] = {xs >> 8, xs & 0xFF, xe >> 8, xe & 0xFF};
    uint8_t y_pos[4] = {ys >> 8, ys & 0xFF, ye >> 8, ye & 0xFF};
    ST77916_Select(display);
    qspi_send_cmd(LCD_CMD_SINGLE, 0x2A, QSPI_ADDRESS_1_LINE, QSPI_DATA_1_LINE, x_pos, 4);
    ST77916_Deselect(display);
    ST77916_Select(display);
    qspi_send_cmd(LCD_CMD_SINGLE, 0x2B, QSPI_ADDRESS_1_LINE, QSPI_DATA_1_LINE, y_pos, 4);
    ST77916_Deselect(display);
    /* 0x2C (RAMWR) is sent by the caller so pixel data follows in the same CS window */
}

void ST77916_Fill(ST77916_Display_t display, uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{
    uint32_t num = (uint32_t)(xe - xs) * (ye - ys);
    ST77916_SetWindow(display, xs, ys, xe - 1, ye - 1);

    static uint16_t fill_buf[360];
    uint16_t color_be = (color >> 8) | (color << 8);
    for (uint32_t i = 0; i < 360; i++) fill_buf[i] = color_be;

    uint32_t remaining = num;
    uint8_t first = 1;
    while (remaining > 0) {
        uint32_t chunk = (remaining > 360) ? 360 : remaining;

        QSPI_CommandTypeDef s = {0};
        s.Instruction      = LCD_CMD_SINGLE;  /* 0x02: 1-line, matches init protocol */
        s.InstructionMode  = QSPI_INSTRUCTION_1_LINE;
        s.Address          = first ? (0x2C << 8) : (0x3C << 8);
        s.AddressSize      = QSPI_ADDRESS_24_BITS;
        s.AddressMode      = QSPI_ADDRESS_1_LINE;
        s.DataMode         = QSPI_DATA_1_LINE;
        s.NbData           = chunk * 2;
        s.DummyCycles      = 0;
        s.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;
        s.DdrMode          = QSPI_DDR_MODE_DISABLE;
        s.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;

        ST77916_Select(display);
        HAL_QSPI_Abort(&hqspi);
        HAL_QSPI_Command(&hqspi, &s, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
        HAL_QSPI_Transmit(&hqspi, (uint8_t *)fill_buf, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
        ST77916_Deselect(display);

        remaining -= chunk;
        first = 0;
    }
}

void ST77916_DrawBuffer(ST77916_Display_t display, uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t *buf)
{
    uint32_t num = (uint32_t)(xe - xs) * (ye - ys);
    ST77916_SetWindow(display, xs, ys, xe - 1, ye - 1);

    QSPI_CommandTypeDef s = {0};
    s.Instruction      = LCD_CMD_SINGLE;
    s.InstructionMode  = QSPI_INSTRUCTION_1_LINE;
    s.Address          = 0x2C << 8;
    s.AddressSize      = QSPI_ADDRESS_24_BITS;
    s.AddressMode      = QSPI_ADDRESS_1_LINE;
    s.DataMode         = QSPI_DATA_1_LINE;
    s.NbData           = num * 2;
    s.DummyCycles      = 0;
    s.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;
    s.DdrMode          = QSPI_DDR_MODE_DISABLE;
    s.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;

    ST77916_Select(display);
    HAL_QSPI_Abort(&hqspi);
    HAL_QSPI_Command(&hqspi, &s, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    HAL_QSPI_Transmit(&hqspi, (uint8_t *)buf, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    ST77916_Deselect(display);
}
