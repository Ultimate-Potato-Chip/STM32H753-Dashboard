# Ford F-100 Digital Gauge Cluster

STM32H753ZI-based digital instrument cluster for a Ford F-100. Replaces the analog gauge cluster with five displays (four round peripheral gauges plus a square center display) driven from CAN/ADC/pulse inputs.

## Hardware

- **MCU:** STM32H753ZI on Nucleo-144 dev board
- **Displays:**
  - 4× ST77916 round 360×360 LCDs (peripheral gauges: fuel, oil pressure, coolant temp, charge voltage) — driven via shared QSPI, individual CS lines
  - 1× NV3052CGRB-based 4" 720×720 round LCD (center display: speedometer, tachometer, high-beam) — 18-bit RGB666 panel driven from LTDC (RGB565 framebuffer → 24-bit LTDC bus, panel reads top 6 bits of each channel) + 3-wire SPI for init. The pixel matrix is 720×720; the visible aperture is circular so the corners are not rendered.
- **Inputs:**
  - Holley Sniper 2 EFI over CAN (FDCAN1, 1 Mbps)
  - ADC: oil pressure, coolant temp, battery voltage, fuel level, headlight dimmer rheostat
  - Pulse: VSS, tachometer (timer input capture)
  - Discrete: high beam, left/right turn signals
- **Other:** I²C EEPROM for calibration storage, USART for Bluetooth phone config UI, USART for debug

## Build

CubeIDE-CMake project. The `.ioc` file is the source of truth for peripheral config; pin assignments and HAL init code are regenerated from CubeMX.

```
cmake --preset default
cmake --build build/Release
```

Flashing/debug is via the STM32 VS Code Extension (`.vscode/launch.json` is committed).

## Project Layout

```
Core/           CubeMX-generated HAL init + main.c
Drivers/        STM32 HAL + CMSIS (vendored)
Display/        Display drivers (st77916.c/h, nv3052c.c/h)
CAN/            Holley Sniper 2 protocol parser (in progress)
Sensors/        ADC sensor reading (in progress)
Inputs/         Pulse input handling (in progress)
Config/         Persistent calibration / settings
cmake/          Toolchain + STM32H7 cmake helpers
```

## Design Notes

[`docs/design.md`](docs/design.md) — living document of architecture decisions (input handling, calibration UI, hardware choices).

## Current Status

- **ST77916 driver:** working — solid color test confirmed
- **NV3052C driver:** debugging an unresolved issue (see below)
- **CAN / ADC / pulse inputs:** peripherals configured, parsers not yet wired

## NV3052C — current unresolved issue

Reference materials (vendor datasheets + manufacturer's golden init sequence) are in [`_REMOVE-AFTER-NV3052C-WORKS/`](_REMOVE-AFTER-NV3052C-WORKS/) — that folder gets deleted once this issue is resolved.

The NV3052CGRB center display is showing a **perfect 50/50 vertical split** when running a background-color-only test (LTDC `BCCR` cycling green→red→blue, layer disabled). One half displays the BCCR color cycling correctly (though somewhat washed-out); the other half shows a garbled "barcode" pattern.

What we've tested:
- SPI init sequence is byte-for-byte the manufacturer's golden init for this exact module (SF-TO400XC-8996A2-N from Saef Technology). Manufacturer init code is in `DataSheets/` reference.
- Tested register `0x23` values (RGB interface control): 0x80 (SYNC+DE), 0xA0 (manufacturer's, DE-only), 0xA2 (alternate), 0x00.
- Tested LTDC timing: HBP 44/120/200, HFP 46/120, PCLK 48 MHz / 38.4 MHz.
- MADCTL `ss` bit flips which half is garbled — confirms the garbled side is whichever bank receives the *late* pixels in each scan line.
- Datasheet confirms the IC has a 2160-channel source driver split into two physical banks: S1–S1080 + S1321–S2400 (gap is unused dummy outputs). The 720-pixel panel maps left half to bank A, right half to bank B.

The split is **invariant to PCLK frequency, HBP, HFP, and `0x23` value**. It moves only with MADCTL `ss`. Pattern persisted across two different Nucleo boards with identical wiring, which makes per-board hardware defect unlikely.

Outstanding theory worth checking: the LTDC layer is currently configured with `ImageWidth=0, FBStartAdress=0, Alpha=0, BlendingFactor1=CA, BlendingFactor2=CA`. We then `__HAL_LTDC_LAYER_DISABLE` + `SRCR_IMR` to disable the layer. But the layer's shadow registers retain the bad config, and we don't know how the LTDC pipeline handles `pitch=0` with the layer technically enabled at any point. The asymmetric BlendingFactor pair (both CA) is also suspicious — if HAL's `LTDC_BLENDING_FACTOR2_CA` actually evaluates to `CA` (not `1-CA`), then with `Alpha=0` the blend math becomes `layer*0 + bg*0 = black`, which could explain BCCR appearing only on the half of the screen the layer "doesn't reach."

Relevant files: [Display/nv3052c.c](Display/nv3052c.c), [Core/Src/main.c](Core/Src/main.c) (search `MX_LTDC_Init`, `pLayerCfg`), [Core/Src/stm32h7xx_hal_msp.c](Core/Src/stm32h7xx_hal_msp.c) (LTDC GPIO + PLL3 config).
