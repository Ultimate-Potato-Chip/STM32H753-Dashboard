#ifndef INPUTS_H
#define INPUTS_H

#include "main.h"
#include <stdint.h>

typedef struct {
    float    speedMph;       /* from VSS — TIM2 CH1 input capture, PA0 */
    float    rpm;            /* from tach — TIM1 CH1 input capture, PE9 */
    uint8_t  leftTurn;       /* PB12 GPIO */
    uint8_t  rightTurn;      /* PB13 GPIO */
    uint8_t  highBeam;       /* PD9  GPIO */
} InputData;

extern InputData inputs;

void Inputs_Init(void);
void Inputs_Update(void);   /* call each main-loop iteration */

#endif /* INPUTS_H */
