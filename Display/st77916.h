#ifndef ST77916_H
#define ST77916_H

#include "main.h"
#include <stdint.h>

#define ST77916_WIDTH  360
#define ST77916_HEIGHT 360

/* QSPI command modes */
#define LCD_CMD_SINGLE  0x02  /* 1-line cmd, 3-byte addr, 1-line data */
#define LCD_CMD_QUAD    0x32  /* 1-line cmd, 3-byte addr, 4-line data */
#define LCD_CMD_QUAD4   0x38  /* 4-line cmd, 3-byte addr, 4-line data */

/* Display select indices */
typedef enum {
    ST77916_DISPLAY_1 = 0,
    ST77916_DISPLAY_2,
    ST77916_DISPLAY_3,
    ST77916_DISPLAY_4,
    ST77916_DISPLAY_ALL
} ST77916_Display_t;

void ST77916_Init(ST77916_Display_t display);
void ST77916_InitAll(void);
void ST77916_Select(ST77916_Display_t display);
void ST77916_Deselect(ST77916_Display_t display);
void ST77916_SetWindow(ST77916_Display_t display, uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye);
void ST77916_Fill(ST77916_Display_t display, uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color);
void ST77916_DrawBuffer(ST77916_Display_t display, uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t *buf);

#endif /* ST77916_H */
