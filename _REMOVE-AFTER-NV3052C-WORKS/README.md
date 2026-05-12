# Delete this whole folder once the NV3052C display works

These are reference materials for debugging the NV3052C center display. They are vendor datasheets and a manufacturer-supplied init sequence, included so a reviewer can verify our init code against the source-of-truth without having to ask for them separately.

When the display is working and the README's "Current Status" section no longer mentions the half-split issue, **delete this entire folder** and remove the corresponding section from the project README.

## Contents

- `NV3052CGRB-Datasheet-V0.2(3).pdf` — NV3052C driver IC datasheet from Shanghai New Vision Microelectronics
- `NV3052CGRB_datasheet.md` — Markdown extraction of the same datasheet (searchable)
- `SF-TO400XC-8996A2-N Specification.pdf` — Saef Technology panel module spec (the 4" 720×720 glass)
- `SF-TO400XC-8996A2-N Specification.md` — Markdown extraction of the same spec
- `SF-TO400XC-8996A2-N initialization code------4.0圆形20220610.txt` — Manufacturer's golden SPI register init sequence for this specific module. The init in `Display/nv3052c.c` is derived from this file.
