#include "inputs.h"
#include "config.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

InputData inputs = {0};

/* -------------------------------------------------------------------------
 * VSS — TIM2 CH1 input capture on PA0
 *
 * TIM2 is 32-bit, clocked at 64 MHz (HSI direct, APB1 timer clock).
 * At 64 MHz tick rate, 32-bit period covers ~67 seconds — far longer than
 * any realistic VSS pulse interval. We can use the raw capture delta as
 * the pulse period without overflow handling.
 *
 * Pulse → MPH:
 *   freq_Hz = VSS_TIM_CLK_HZ / period_ticks
 *   mph     = freq_Hz × 3600 / pulses_per_mile
 *           = (VSS_TIM_CLK_HZ × 3600) / (period_ticks × pulses_per_mile)
 * -------------------------------------------------------------------------*/
#define VSS_TIM_CLK_HZ          64000000U
#define VSS_TIMEOUT_MS          2000U       /* no pulse for 2s → 0 mph */
#define VSS_MIN_PERIOD_TICKS    100U        /* < 100 ticks (~1.5 µs) is noise */
#define VSS_EMA_ALPHA           0.2f        /* smoothing — higher = snappier, jittier */

static volatile uint32_t vssLastCaptureTick = 0;
static volatile uint32_t vssLastPeriodTicks = 0;
static volatile uint32_t vssLastPulseMs     = 0;
static volatile uint8_t  vssHaveFirstCap    = 0;

void Inputs_Init(void)
{
    /* TIM2 input capture for VSS. _IT variant enables the capture interrupt. */
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

    /* CubeMX did not enable the TIM2 NVIC line because the .ioc has TIM2 as
     * plain input capture without the "global interrupt" checkbox set. Enable
     * here so HAL_TIM_IC_CaptureCallback actually fires.
     * TODO: move this into CubeMX so it survives regeneration. */
    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    /* Tach (TIM1 CH1, PE9) — polling for now, TODO same pattern as VSS */
    HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_1);
}

void Inputs_Update(void)
{
    /* --- VSS: convert latest captured period to MPH ------------------- */
    uint32_t periodTicks;
    uint32_t lastPulseMs;

    __disable_irq();
    periodTicks = vssLastPeriodTicks;
    lastPulseMs = vssLastPulseMs;
    __enable_irq();

    uint32_t pulsesPerMile = calibration.vssPulsesPerMile;

    if (periodTicks == 0 || pulsesPerMile == 0 ||
        (HAL_GetTick() - lastPulseMs) > VSS_TIMEOUT_MS) {
        /* Stopped, never moved, or uncalibrated. */
        inputs.speedMph = 0.0f;
    } else {
        float instantMph = ((float)VSS_TIM_CLK_HZ * 3600.0f) /
                           ((float)periodTicks * (float)pulsesPerMile);
        /* Exponential moving average — keeps the speedo needle from jittering
         * on every pulse but still responsive enough to feel "live." */
        inputs.speedMph = VSS_EMA_ALPHA * instantMph +
                          (1.0f - VSS_EMA_ALPHA) * inputs.speedMph;
    }

    /* --- Tach: TODO same pattern, once we settle on the signal source --- */

    /* --- GPIO inputs -------------------------------------------------- */
    inputs.leftTurn  = HAL_GPIO_ReadPin(TURN_LEFT_IN_GPIO_Port,  TURN_LEFT_IN_Pin)  == GPIO_PIN_SET;
    inputs.rightTurn = HAL_GPIO_ReadPin(TURN_RIGHT_IN_GPIO_Port, TURN_RIGHT_IN_Pin) == GPIO_PIN_SET;
    inputs.highBeam  = HAL_GPIO_ReadPin(HIGHBEAM_IN_GPIO_Port,   HIGHBEAM_IN_Pin)   == GPIO_PIN_SET;
}

/* HAL invokes this on every captured edge across any timer/channel.
 * Filter on instance + channel to identify which input fired. */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        uint32_t now = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        if (vssHaveFirstCap) {
            /* 32-bit unsigned subtraction handles counter wraparound naturally. */
            uint32_t period = now - vssLastCaptureTick;
            if (period >= VSS_MIN_PERIOD_TICKS) {
                vssLastPeriodTicks = period;
                vssLastPulseMs     = HAL_GetTick();
            }
        } else {
            vssHaveFirstCap = 1;
        }
        vssLastCaptureTick = now;
    }
    /* TODO: tach on TIM1 here once we wire that up */
}

/* TIM2 global interrupt vector. The weak default in startup_*.s does nothing;
 * this strong definition routes the IRQ to HAL, which dispatches to the
 * capture callback above. */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}
