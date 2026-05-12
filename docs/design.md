# Design Notes

Living document. Captures design decisions made during development so they don't get lost in chat history. Update as decisions evolve.

---

## Grounding philosophy (applies to all sensors)

**DECISION PENDING** — two viable topologies. Decide before PCB layout / connector selection.

### Option A: Shared low-reference terminal (likely choice — fewer pins, simpler install)

Single `SENSOR_GND` terminal on the harness that customers connect once. All resistive sender grounds (fuel, oil, coolant) tie into this one terminal via a junction in the harness or back at the dashboard. Inside the dashboard, that single terminal lands on the PCB's signal-ground plane.

- Pin count: ~5 pins for 4 sensors (4 SIG + 1 GND)
- Failure mode: if SENSOR_GND develops a bad connection, all sensors affected at once — easy to diagnose
- Matches Autometer Invision and many factory cluster topologies
- Lower BOM cost (smaller connector)

### Option B: Per-sensor dedicated grounds (more pins, better fault isolation)

Each sensor on the harness gets its own ground terminal pin (`FUEL_GND`, `OIL_GND`, `COOLANT_GND`). Inside the dashboard, all of them still converge on the same single signal-ground plane on the PCB — the "star point" is on the PCB, not at the customer end.

- Pin count: ~8 pins for 4 sensors (4 SIG + 4 GND)
- Failure mode: a bad ground wire affects only one sensor — others remain accurate
- Matches AEM CD-5 / CD-7 topology
- Better noise isolation but the practical difference is small (~2 mV at our currents)

### Shared elements regardless of topology

- All grounds converge on **one unified signal-ground plane** inside the dashboard. PCB layout uses a single contiguous copper pour; no splits under analog signal traces.
- The PCB signal ground bonds to chassis/battery ground at exactly **one** carefully chosen point — typically right at the dashboard's main power input terminal block. No secondary bonds.
- The 12V switched excitation rail for resistive senders is shared across sensors — one fused 12V_SW on the PCB, individually pulled up to each sensor signal pin via that sensor's own R_known. Each input has its own TVS clamp and analog low-pass filter, so a fault on one sensor wire can't propagate to another.
- **Dimmer input** does NOT need a dedicated ground — we're measuring the voltage on the existing instrument-panel illumination wire (already in the harness running from the headlight switch's rheostat to the panel bulbs). That signal is referenced to chassis via the bulbs' mounts; we read it relative to our dashboard GND.

### Customer install instruction (manual, both options)

Bond the dashboard's main GND terminal to the **engine block ground stud** (where the battery negative cable terminates), not random sheet metal. This puts the dashboard at the same potential as the alternator and battery, which is the cleanest reference in the car.

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

**One terminal on the harness, labeled `VSS`.** Customer wires whatever they have to this single pin; the dashboard takes care of conditioning whatever signal they feed it. The signal feeds a **MAX9924 signal conditioner** (or equivalent: MAX9925/26/27, NCV1124) configured for single-ended operation, → MCU input capture pin (`PA0` on `TIM2_CH1`).

**Three supported customer wiring scenarios:**

1. **Hall-effect speed sensor (3-wire active sensor)**
   - Sensor signal wire → dashboard `VSS` terminal
   - Sensor +V (typ. +12V) → any switched-ignition 12V source in the vehicle (factory accessory wire, fuse box, key switch — customer's choice, not the dashboard's responsibility)
   - Sensor GND → chassis ground at sensor body

2. **ECU VSS output (Holley Sniper, Megasquirt, GM ECU pulse output, etc.)**
   - ECU speed output wire → dashboard `VSS` terminal
   - That's it. Single wire — the ECU provides the signal referenced to its own ground (shared with chassis).

3. **Variable Reluctance sensor (2-wire passive sensor, e.g., T56 trans speedo pickup)**
   - One sensor wire → dashboard `VSS` terminal
   - Other sensor wire → chassis ground (typically at the sensor body or a nearby clean ground point)
   - The MAX9924 handles the AC sine wave, adaptive threshold, and converts to digital pulses regardless of speed-varying amplitude.

**Why one terminal instead of three (`VSS_HI` / `VSS_LO` / `VSS_PWR`):** Simpler install instructions, fewer wires for the customer to run, fewer pins on the harness connector, fewer support calls. The theoretical noise advantage of providing a dedicated dash-side ground reference for VR sensors is negligible in practice — the MAX9924's adaptive threshold easily handles single-ended signals over typical automotive wire runs. This matches the customer-facing simplicity of Holley/AEM products: "connect your speed wire here, done."

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

- TIM2 CH1 (PA0) is configured for input capture in CubeMX. The HAL `HAL_TIM_IC_CaptureCallback` fires on each rising edge — measure time delta between captures → period → frequency.
- TIM2 is 32-bit running at 64MHz tick rate (HSI direct, APB1 timer clock). 32-bit counter at 64MHz wraps every ~67 seconds, far longer than any realistic VSS pulse interval, so no overflow handling is needed.
- Speed averaging: implementation uses simple moving average over the last N pulse periods, with N exposed as user calibration parameter `pulsesToAverage`. Holley convention is "at least 1/4 of your tooth count" (e.g., 5+ pulses for a T56's 17 PPR), more for smoother needle.
- Pulse timeout: if no edge for >2 seconds, force speed to 0. Prevents stale readings when the vehicle stops.

### Reference example: T56 with 3.42 rear gear, 275/60R15 tires

- **Direct speedo input mode (dashboard reads sensor directly):**
  - VSS Teeth: 17 (T56 transmission output shaft pickup)
  - Gear: 3.42 (rear axle ratio, no transformation)
  - Tire Diameter: 27.99" (calculated from 275/60R15)
  - Pulses to average: 5 or more (1/4 of 17)
  - Effective `pulsesPerMile = (63360 / (tire × π)) × gear × ppr = (63360 / 87.94) × 3.42 × 17 ≈ 41,879`

- **Trans ECU output mode (dashboard transforms pulses for a Holley ICF or similar that expects 40 PPR):**
  - Output a pulse train representing 40 PPR instead of the input's 17 PPR
  - This is the future VSS-output feature (PWM timer driving an output pin)
  - The dashboard's display reading uses the original 17 PPR for accuracy
  - The output frequency is calculated from the dashboard's known MPH

---

## Fuel Level

### Design goal
Single labeled "FUEL" input on the harness. Accept any resistive fuel level sender (factory or aftermarket) without requiring the customer to know what their sender's resistance range is.

### Sender source options (user-selectable in calibration UI)

1. **Stock profile** — preset endpoints for common factory senders. User picks their vehicle's profile from a dropdown:
   - GM (early): 0Ω full, 90Ω empty
   - GM (modern): 40Ω full, 250Ω empty
   - Ford: 73Ω full, 8-16Ω empty
   - Mopar/Chrysler: 10Ω full, 73Ω empty
   - Aftermarket American standard: 240Ω empty, 33Ω full
2. **Two-point calibration** — user empties tank, taps "Set 0%", fills tank, taps "Set 100%". System stores both raw ADC values and interpolates. Handles any sender range AND direction (rising or falling Ω with fuel).
3. **Multi-point calibration** *(future / v2)* — user adds intermediate points at ¼, ½, ¾ tank. Improves accuracy on senders with nonlinear float-arm geometry.

### Hardware path

12V excitation (switched ignition, not constant battery — avoids parasitic drain through sender when key is off) → known pull-up resistor → ADC pin (currently `PC5` on `ADC1`) → fuel sender (variable resistor to chassis ground via tank).

```
12V_SW ── R_known (1kΩ) ──┬──── ADC pin (PC5)
                          │
                          ├──[ TVS/Schottky clamp to 3.3V ]── overvoltage protection
                          │
                          ├──[ 100nF to signal GND ]── slosh filter
                          │
                          ╰── FUEL_SIG terminal ── R_sender ── return path:
                                                               (a) FUEL_GND terminal  ← recommended
                                                                   (tied to dashboard signal GND plane)
                                                               (b) chassis ground via sender body
                                                                   (works, not recommended)
```

**Why 12V excitation:** classic cars are notorious for chassis-ground offset issues. A 200–500mV difference between chassis ground and dashboard signal ground is common, especially with corroded grounds, rusty body mounts, or one-wire alternator setups. With 3.3V excitation, that's a 6–15% measurement error in your fuel gauge. With 12V excitation, the same ground offset is only a 2–4% error. Higher excitation current (10mA vs 2mA) also cuts through wiper contact noise on aging senders.

**Two-wire (recommended) vs. one-wire (supported but unwarranted):**

The harness includes a dedicated **FUEL_GND terminal** that ties directly to the dashboard's signal ground plane. This is the **recommended** way to wire any fuel sender:

- **Two-wire senders** (some aftermarket, some marine, rare factory): one wire to the FUEL signal terminal, the other to FUEL_GND. This is the clean, accurate, low-noise configuration we engineer toward.
- **One-wire senders with a retrofitted ground wire** (recommended for classic-car installs): customer runs a small-gauge wire from the FUEL_GND terminal to the sender body, mounting flange, or a clean tank-mounted ground stud. This converts a factory one-wire sender into a clean two-wire installation. Five minutes of work, eliminates ground-offset error.
- **One-wire senders relying on chassis ground** (technically supported, not recommended): the FUEL_GND terminal is left disconnected and the sender body returns to chassis via the tank's mechanical mounting. This will work — the two-point custom calibration absorbs most of the static ground offset — but accuracy and stability cannot be guaranteed. Documented in the install manual:

> **Disclaimer (install manual):** While one-wire senders relying on chassis ground are technically supported, the accuracy and reliability of the reading cannot be guaranteed without a dedicated ground wire. For best results, run a small-gauge wire from the FUEL_GND terminal to the sender body or tank mounting flange.

This framing protects the product's accuracy reputation (warranty calls about jumpy fuel gauges become "did you connect FUEL_GND?") while still letting customers do a clean factory-look install if they really insist.

**Component sizing:**
- `R_known = 1kΩ` — sized so the ADC pin never exceeds ~2.4V even at the highest expected sender resistance (250Ω GM modern): `12V × 250/1250 = 2.4V`. Plenty of headroom under the 3.3V max.
- TVS or Schottky clamp diode from ADC pin to 3.3V — protects the MCU input in case of sender wire open-circuit (which would otherwise pull the ADC pin to 12V via the pull-up).
- 100nF cap to ground — first-stage analog low-pass filter to suppress sloshing transients before they reach the ADC. Time constant with the divider Thevenin impedance (~700Ω) is ~70µs — fast enough not to bias the reading but kills sharp transients.
- Software-side digital filter handles longer-term smoothing (see below).

**Current draw at the sender:** worst case 12V / (1kΩ + 10Ω) ≈ 12mA. Power dissipated in a low-resistance sender (10Ω full): 1.4mW — well within any fuel sender's rating. No sender damage concern.

### Software path

1. Read raw 12-bit ADC value periodically (every 100ms or so — fast enough, not wasting cycles).
2. Apply low-pass filter with ~10–30 second time constant. (For an exponential moving average: `filtered = filtered + α × (raw - filtered)` where `α ≈ 0.001` at 100ms sample rate gives ~10s settling time.)
3. Map filtered ADC value to 0–100% based on calibration mode:
   - **Stock profile or two-point cal:**
     ```
     percent = (adc_filtered - adc_empty) × 100 / (adc_full - adc_empty)
     clamp to [0, 100]
     ```
   - **Multi-point cal (future):** linear interpolation between adjacent calibration points.
4. Optional: a second slower damping filter (~3–5 seconds) applied to the *output* percent before driving the gauge needle, so the visible needle motion is gentle even if the underlying value updates.

### Calibration UI (Bluetooth phone app)

```
Sender source: [ Stock profile | Custom calibration ]

  if Stock profile:
    Profile: [ GM early | GM modern | Ford | Mopar | Aftermarket | … ]

  if Custom calibration:
    [ Set 0%  (tank empty) ]      ← captures current ADC reading
    [ Set 100% (tank full) ]      ← captures current ADC reading
    Current reading: 47%          ← live display so user can verify
    [ Reset calibration ]
```

Stored values: just two int16s per calibration slot (adc_empty, adc_full). Persist to EEPROM along with the rest of the user config.

### Implementation notes

- Calibration captures the FILTERED ADC value, not the raw sample, so the captured endpoint isn't biased by a single noisy sample.
- If `adc_full == adc_empty` (user error or uncalibrated), fall back to "—" on the gauge rather than dividing by zero.
- Sloshing is severe enough during cornering that even with the filter you may see the gauge dip then recover. This is acceptable and matches factory gauge behavior. The damping filter on the output keeps the *visible* needle smooth.

---

## Tachometer

TODO — same pattern as VSS but reading from PA0 / TIM2 CH1. Calibration field: pulses per crankshaft revolution (typically 4 for an 8-cylinder spark output, 2 for a 4-cylinder, but varies by source — coil negative, distributor pickup, ECU tach output).

## Oil Pressure

### v1 scope: single OIL terminal, two sender types

Customer's `OIL` terminal accepts either of the two common factory configurations on classic vehicles. Voltage transducers (0.5–4.5V active sensors) are deferred to v2 as a separate hardware input pin — those customers typically have aftermarket ECUs with CAN and can read oil pressure from CAN instead.

| Type | Wiring | Reading |
|---|---|---|
| `OIL_SENDER_SWITCH` | 1-wire pressure switch, body grounds via engine block. Switch closes to ground when oil pressure drops below ~5 PSI. | Binary state only. Drives "low oil pressure" warning indicator. No PSI gauge value. |
| `OIL_SENDER_RESISTIVE` | 1- or 2-wire variable resistance sender (GM 0–90Ω, Ford 0–73Ω, etc.). Body grounds via engine block on 1-wire, dedicated wire on 2-wire. | Voltage divider + ADC → resistance → linear map to PSI. |

Same voltage-divider topology as fuel for the resistive case. The switch case re-uses the same ADC pin and pull-up — the switch grounds the line when closed, leaves it pulled high when open.

### Calibration UI

```
Oil sender type: [ Pressure switch | Resistive sender ]

  if Resistive sender:
    Ohms at 0 PSI:    [    ]  (default 0)
    Ohms at max PSI:  [    ]  (default 90)
    Max PSI:          [    ]  (default 100)
```

Worth providing presets here too (GM, Ford, AutoMeter common ranges) as a dropdown above the manual values to skip the lookup for most customers.

---

## Coolant Temp

### v1 scope: thermistor with preset library + custom curve

All factory classic-car coolant senders are NTC thermistors — resistance decreases as temperature rises, **non-linearly**. Two-point linear calibration would be noticeably wrong at intermediate temperatures, so we need a curve.

### Calibration approach

Three modes selectable in the app:

1. **Preset profile** (default for most customers): pick from a built-in curve library (currently GM and Ford; more to be added). Each preset is a 5-point R/T table that the firmware interpolates between.
2. **Custom curve**: user enters 2–5 R/T pairs via the app. For owners of non-standard senders, racing applications, etc.
3. **Reverse-engineered preset** (added by us, not the customer): see workflow below.

### How we add new presets to the library

Procedure for characterizing an unknown sender to add it as a built-in preset:

1. **Get a reference sender with a documented manufacturer curve** (e.g., a fresh GM AC Delco sender with its published R/T table).
2. **Set up a water bath** — pot of water on a hot plate with a precision thermocouple meter.
3. **Submerge both the reference sender and the unknown sender** at the same depth, ensuring good thermal contact between them.
4. **Heat the bath through the operating range** (e.g., room temp → 32°F (with ice) → 100°F → 160°F → 212°F (boiling)), pausing at each checkpoint for thermal equilibration.
5. **At each checkpoint**, read:
   - Water temperature (thermocouple)
   - Reference sender resistance (ohmmeter) — sanity-check against published curve
   - Unknown sender resistance (ohmmeter)
6. **Tabulate the unknown sender's R/T pairs** into a `CoolantPresetCurve` struct and add to `sensors.c`.
7. **Ship the new preset** as an option in the next firmware release.

This is exactly how aftermarket gauge manufacturers (Auto Meter, VDO) characterize the senders they include in their app dropdowns.

### Implementation notes

- 2200Ω divider pull-up handles the full thermistor range (low hundreds of Ω hot, tens of kΩ cold) without saturating either rail.
- Linear interpolation between adjacent table points. Good enough for ±1–2°F accuracy at intermediate temps if cal points are spaced reasonably (every 50–80°F).
- Off-curve readings (e.g., sensor disconnected → infinite R → register reads near rail voltage) are clamped to "below coldest point" or "above hottest point" rather than reported as wild garbage values.

---

## Future / Roadmap

Captured here so it doesn't get lost in chat history. Not committed to a release.

### Community-customizable gauge faces

Idea: let owners design their own gauge faces and share them. Holley/AEM ship a fixed set of themes; nobody does open community sharing yet. Could be a real product differentiator.

Implementation tiers, easy → hard:

1. **Theme selection** — multiple built-in gauge faces compiled into firmware, user picks one in the app. Lowest engineering cost. Doesn't need community infrastructure but doesn't have "make it yours" appeal either.
2. **Configurable display assignment** — user picks which gauge appears on which of the 4 round displays (e.g., "swap oil pressure and fuel"). Pure config, no asset work.
3. **Custom asset upload** — user uploads gauge face bitmap, needle bitmap, colors, redline marks via the app. Stored in external flash or SD card. Needs an asset bundle format and reasonable rendering performance.
4. **Fully scripted gauges** — small DSL or scripting engine letting users define gauge logic, animations, custom data combinations. Significant engineering effort.

Realistic v2 target: tiers 1 + 2 (themes + display assignment). Tier 3 if external storage is added. Tier 4 is probably never worth the complexity vs. just shipping more built-in themes.

**Storage math:** STM32H753 has 2MB internal flash, ~37KB used today. A single uncompressed RGB565 720×720 gauge face = 1 MB. So 1–2 high-res themes fit internally without external storage, more with compression (PNG, RLE, or palette mode for simple graphics).

**Community infrastructure:** if we ever want a "theme marketplace" experience, the easiest path is a GitHub-style repo where users submit themes via PR + a curated index file the Bluetooth app pulls from. No backend infrastructure required initially.

### Additional CAN sources for oil pressure (and others)

Holley Sniper 2 doesn't broadcast oil pressure on its CAN — but GM and Ford factory ECUs do, on their proprietary CAN messages. When we want to support OEM CAN integration (LT1 swaps, Coyote swaps, Vortec swaps that retain the original ECU), add the appropriate CAN IDs:

- **GM Gen V LT1/LT4 (E92 ECM):** oil pressure on proprietary CAN — research the specific message ID per platform.
- **Ford Coyote 5.0L (Control Pack):** oil pressure broadcast on the Ford CAN — research the ID.
- **Standard OBD-II:** oil pressure is *not* a standard PID (only oil temp, PID 0x5C, is). Don't rely on it.

Each one needs a new `CAN_ID_*` constant in `holley_can.h` (rename the file when we add multi-OEM support — `oem_can.c`?) and a case in the dispatch switch. The `Source_*()` accessor architecture handles the rest automatically.

### Voltage transducers for oil pressure (v2 hardware)

0.5–4.5V transducers (Bosch, Honeywell, AutoMeter premium) need a different input topology than the resistive/switch case (no pull-up, just a 2:3 divider to scale 5V → 3.3V). Add a separate `OIL_TRANS` input pin on the harness in the next PCB revision. Same `OilSenderType` enum gets a new value `OIL_SENDER_TRANSDUCER`.

---

## Battery Voltage / Dimmer

Both read via a resistor divider (100k / 33k for a ~4:1 ratio, scales 0–15V battery rail down to 0–3.3V ADC range). Same divider topology for both; just different ADC channels.

Battery voltage is straightforward — multiply ADC voltage by the divider ratio for the rail voltage. Generates a `lowBattery` warning if engine is running and voltage drops below 12.5V.

Dimmer is the voltage on the factory panel illumination wire (output of the headlight switch rheostat). Customer-specific calibration captures their dimmer's voltage range during install — they sweep the rheostat to min and max while watching the captured value in the app, then save those bounds. Display brightness PWM maps linearly between those two voltage endpoints.
