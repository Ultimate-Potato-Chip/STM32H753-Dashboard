#include "inputs.h"
#include "config.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

InputData inputs = {0};

void Inputs_Init(void)
{
    HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_1);   /* tach */
    HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_1);   /* VSS  */
}

void Inputs_Update(void)
{
    /* TODO: compute period from consecutive input-capture values to derive frequency,
     * then convert:
     *   VSS:  freq / calibration.vssPulsesPerMile * 3600 → speedMph
     *   Tach: freq * 60 / (cylinders/2) → rpm  (depends on signal source — one pulse/rev?) */

    inputs.leftTurn  = HAL_GPIO_ReadPin(TURN_LEFT_IN_GPIO_Port,  TURN_LEFT_IN_Pin)  == GPIO_PIN_SET;
    inputs.rightTurn = HAL_GPIO_ReadPin(TURN_RIGHT_IN_GPIO_Port, TURN_RIGHT_IN_Pin) == GPIO_PIN_SET;
    inputs.highBeam  = HAL_GPIO_ReadPin(HIGHBEAM_IN_GPIO_Port,   HIGHBEAM_IN_Pin)   == GPIO_PIN_SET;
}
