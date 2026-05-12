# Design Notes

Living document. Captures design decisions made during development so they don't get lost in chat history. Update as decisions evolve.

---

## Vehicle Speed (VSS)

### Design goal
A single labeled "VSS" terminal on the customer's wiring harness that accepts *whatever* speed signal they have available — without making them think about what their sensor electrical type is.

### Speed source options (user-selectable in calibration UI)

1. **CAN bus** — speed comes from the engine ECU over the existing FDCAN1 connection (e.g., Holley Sniper 2). No VSS wiring needed.
2. **VSS pulse input** — pulse train from any of:
   - Factory mechanical/Hall-effect VSS in transmission
   - Reluctor (variable-reluctance) sensor in transmission
   - Aftermarket ECU pulse output (e.g., GM ECU VSS output, typically 4000 or 8000 PPM)
   - Cable-driven adapter that converts mechanical rotation to pulses

### Hardware path (input)

VSS terminal → **MAX9924 signal conditioner** (or equivalent: MAX9925/26/27, NCV1124) → MCU input capture pin (currently `PE9` on `TIM1_CH1`).

The MAX9924 is purpose-built to accept both VR (variable reluctance, AC sine wave) and Hall-effect (DC square wave) inputs in the same chip and produce a clean 3.3V digital output for the MCU. This means:

- One input pad on the harness for any sensor type
- No customer-side jumper or mode switch
- Robust over the full speed range (idle to highway speeds)

**Alternatives considered and rejected:**
- *LM1815 only:* designed for VR sensors only — Hall square waves usually pass but the adaptive threshold mistracks. Not robust for a product.
- *LM393 comparator with two-path front-end:* cheaper but more parts and requires customer-side mode awareness.

### Hardware path (output, future)

Not implemented yet. If/when needed (to drive cruise control modules, aftermarket TCMs, or feed an external gauge cluster), allocate an unused timer (TIM3/4/5/8 — all free) in PWM mode on any available GPIO. Output frequency = `mph × PPM / 3600`. Drive through a level shifter (open-collector to 5V or 12V depending on downstream consumer).

### Calibration UI (Bluetooth phone app)

The user-facing config screen for speed has three layers:

```
Speed source: [ CAN bus | VSS pulse ]

  if VSS pulse selected:

  Calibration mode: [ ECU pulse output | Calculate from drivetrain ]

    if ECU pulse output:
      Pulses per mile: [    ]    (e.g., 8000 for standard GM ECU)

    if Calculate from drivetrain:
      Trans output PPR:  [    ]   (T56 = 17, TKO = 16, 4L60E = varies)
      Rear axle ratio:   [    ]   (e.g., 3.55, 3.73, 4.10)
      Tire diameter:     [    ]   inches  (or enter tire size code)
```

### Speed math

If user selected **CAN**: use the speed value directly from the ECU CAN message. No calibration math required (the ECU has already done it).

If user selected **VSS pulse + ECU pulse output mode**:
```
MPH = (measured_pulse_Hz × 3600) / pulses_per_mile
```

If user selected **VSS pulse + Calculate from drivetrain**:
```
tire_revs_per_mile  = 63360 / (tire_diameter_inches × π)
pulses_per_mile     = tire_revs_per_mile × rear_ratio × trans_PPR
MPH                 = (measured_pulse_Hz × 3600) / pulses_per_mile
```

Both VSS-pulse modes reduce to the same final formula — they just differ in whether the user supplies `pulses_per_mile` directly or as three factors that get multiplied together.

### Implementation notes

- TIM1 CH1 (PE9) is configured for input capture in CubeMX. The HAL `HAL_TIM_IC_CaptureCallback` fires on each rising edge — measure time delta between captures → period → frequency.
- TIM1 is 16-bit (Period = 65535) with no prescaler. At HSI 64MHz that wraps every ~1ms, which is plenty fast for the highest expected VSS frequencies (~1kHz at 80mph with 8000 PPM ÷ 60 mph→min conversion). Implement overflow handling so very low speeds (long pulse periods) still work.
- Filter the per-pulse speed reading with a short moving average (~5–10 samples) before driving the speedo display, otherwise the needle will jitter on each pulse.

---

## Tachometer

TODO — same pattern as VSS but reading from PA0 / TIM2 CH1. Calibration field: pulses per crankshaft revolution (typically 4 for an 8-cylinder spark output, 2 for a 4-cylinder, but varies by source — coil negative, distributor pickup, ECU tach output).
