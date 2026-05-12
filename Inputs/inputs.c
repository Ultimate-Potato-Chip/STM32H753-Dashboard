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
#define VSS_EMA_ALPHA           0.2f

static volatile uint32_t vssLastCaptureTick = 0;
static volatile uint32_t vssLastPeriodTicks = 0;
static volatile uint32_t vssLastPulseMs     = 0;
static volatile uint8_t  vssHaveFirstCap    = 0;

/* -------------------------------------------------------------------------
 * Tachometer — TIM1 CH1 input capture on PE9
 *
 * TIM1 is 16-bit. APB2 timer clock = 64 MHz. We set the prescaler to 999
 * (divider 1000) here so the timer ticks at 64 kHz — 65,536 ticks covers
 * ~1024 ms, which is comfortably longer than any realistic pulse interval
 * from cranking RPM (~200 rpm) to redline (~8000 rpm) on any signal source
 * (coil-negative single-pulse, distributor pickup, ECU tach output).
 *
 * Pulse → RPM:
 *   freq_Hz = TACH_TIM_CLK_HZ / period_ticks
 *   rpm     = freq_Hz × 60 / pulses_per_rev
 * -------------------------------------------------------------------------*/
#define TACH_PRESCALER          999U
#define TACH_TIM_CLK_HZ         64000U      /* 64 MHz / (TACH_PRESCALER + 1) */
#define TACH_TIMEOUT_MS         2000U       /* no pulse → 0 rpm */
#define TACH_MIN_PERIOD_TICKS   5U          /* ignore impossibly fast pulses */
#define TACH_EMA_ALPHA          0.35f       /* snappier than VSS — tach should feel live */

static volatile uint16_t tachLastCaptureTick = 0;
static volatile uint16_t tachLastPeriodTicks = 0;
static volatile uint32_t tachLastPulseMs     = 0;
static volatile uint8_t  tachHaveFirstCap    = 0;

void Inputs_Init(void)
{
    /* TIM2 (VSS) — interrupt-driven input capture. */
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

    /* TIM1 (tach) — set prescaler at runtime so the 16-bit counter covers
     * the slow-pulse case (cranking RPM) without overflow. CubeMX still
     * generates Prescaler=0; this overrides it. TODO: set in the .ioc. */
    __HAL_TIM_SET_PRESCALER(&htim1, TACH_PRESCALER);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);

    /* CubeMX didn't enable NVIC for either timer (input-capture-only timers
     * without the "global interrupt" checkbox set in the .ioc). Enable both
     * manually so HAL_TIM_IC_CaptureCallback actually fires.
     * TODO: move into CubeMX so it survives regeneration. */
    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    HAL_NVIC_SetPriority(TIM1_CC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
}

void Inputs_Update(void)
{
    uint32_t now = HAL_GetTick();

    /* --- VSS: latest captured period → MPH ---------------------------- */
    {
        uint32_t periodTicks;
        uint32_t lastPulseMs;
        __disable_irq();
        periodTicks = vssLastPeriodTicks;
        lastPulseMs = vssLastPulseMs;
        __enable_irq();

        uint32_t ppm = calibration.vssPulsesPerMile;
        if (periodTicks == 0 || ppm == 0 || (now - lastPulseMs) > VSS_TIMEOUT_MS) {
            inputs.speedMph = 0.0f;
        } else {
            float instantMph = ((float)VSS_TIM_CLK_HZ * 3600.0f) /
                               ((float)periodTicks * (float)ppm);
            inputs.speedMph = VSS_EMA_ALPHA * instantMph +
                              (1.0f - VSS_EMA_ALPHA) * inputs.speedMph;
        }
    }

    /* --- Tach: latest captured period → RPM --------------------------- */
    {
        uint16_t periodTicks;
        uint32_t lastPulseMs;
        __disable_irq();
        periodTicks = tachLastPeriodTicks;
        lastPulseMs = tachLastPulseMs;
        __enable_irq();

        uint8_t ppr = calibration.tachPulsesPerRev;
        if (periodTicks == 0 || ppr == 0 || (now - lastPulseMs) > TACH_TIMEOUT_MS) {
            inputs.rpm = 0.0f;
        } else {
            /* freq_Hz × 60 / ppr; combined: (64000 × 60) / (period × ppr) */
            float instantRpm = ((float)TACH_TIM_CLK_HZ * 60.0f) /
                               ((float)periodTicks * (float)ppr);
            inputs.rpm = TACH_EMA_ALPHA * instantRpm +
                         (1.0f - TACH_EMA_ALPHA) * inputs.rpm;
        }
    }

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
            uint32_t period = now - vssLastCaptureTick;
            if (period >= VSS_MIN_PERIOD_TICKS) {
                vssLastPeriodTicks = period;
                vssLastPulseMs     = HAL_GetTick();
            }
        } else {
            vssHaveFirstCap = 1;
        }
        vssLastCaptureTick = now;
        return;
    }

    if (htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        uint16_t now = (uint16_t)HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        if (tachHaveFirstCap) {
            uint16_t period = now - tachLastCaptureTick;  /* 16-bit wraparound natural */
            if (period >= TACH_MIN_PERIOD_TICKS) {
                tachLastPeriodTicks = period;
                tachLastPulseMs     = HAL_GetTick();
            }
        } else {
            tachHaveFirstCap = 1;
        }
        tachLastCaptureTick = now;
        return;
    }
}

/* TIM2 global IRQ — routes to HAL for VSS capture. */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

/* TIM1 capture/compare IRQ — routes to HAL for tach capture.
 * Note: TIM1 splits its interrupts across multiple vectors
 * (UP/CC/BRK/TRG). Capture events fire on TIM1_CC_IRQn. */
void TIM1_CC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
}
