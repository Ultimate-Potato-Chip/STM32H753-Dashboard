#ifndef NV3052C_H
#define NV3052C_H

#include "main.h"
#include <stdint.h>

#define NV3052C_WIDTH  720
#define NV3052C_HEIGHT 720

void     NV3052C_Init(void);
void     NV3052C_Update(void);          /* diagnostic color cycle — call from main loop */
void     NV3052C_SetColor(uint16_t color_rgb565);

#endif /* NV3052C_H */
