# Capabilities & Reality-Based Verification

This document provides a breakdown of all features supported by **RogueByte PS4 Controller Lab**, along with their official **Reality-Based Engineering** classification.

---

## 1. Reality-Based Classification Schema

Every feature in the application is audited against real PS4 hardware capabilities and classified into one of four statuses:

- **`REAL`**: Implemented using real PS4 hardware interfaces or low-level APIs. Does not simulate or mock results.
- **`PARTIAL`**: Realistically implemented within the limits of unprivileged user-mode execution.
- **`MOCK`**: Emulated for development or testing harness purposes (explicitly labeled).
- **`NOT SUPPORTED`**: Declared unsupported due to hardware or security sandbox boundaries. Displayed as `NOT AVAILABLE` in the UI.

---

## 2. Comprehensive Subsystem Audit

### A. Gamepad Enumeration & HID Discovery
- **Status**: `REAL`
- **Methodology**: Interfaces with `scePadOpen` and `sceUserServiceGetInitialUser`.
- **Supported Models**:
  - Sony DualShock 4 Gen 1 (CUH-ZCT1 series, PID `0x05C4`)
  - Sony DualShock 4 Gen 2 (CUH-ZCT2 series, PID `0x09CC`)
- **Reported Data**: Connection state, USB vs Bluetooth interface, Vendor ID (`0x054C`), Product ID (`0x05C4`/`0x09CC`).

### B. 18-Button Digital & Pressure Matrix
- **Status**: `REAL`
- **Buttons Tested**:
  1. Cross (✕)
  2. Circle (◯)
  3. Square (□)
  4. Triangle (△)
  5. D-Pad Up
  6. D-Pad Down
  7. D-Pad Left
  8. D-Pad Right
  9. L1 (Left Shoulder)
  10. R1 (Right Shoulder)
  11. L2 (Digital actuation threshold)
  12. R2 (Digital actuation threshold)
  13. L3 (Left Stick click)
  14. R3 (Right Stick click)
  15. Share
  16. Options
  17. PS Button
  18. Touchpad Click
- **Trigger Analog Pressure**: Real 8-bit analog reporting (0–255) for L2 and R2.

### C. Joystick Telemetry & Reticles
- **Status**: `REAL`
- **Methodology**: Polled at 250Hz over USB / Bluetooth.
- **Output Metrics**:
  - Raw axes (0–255 with neutral center at ~128).
  - Normalized Cartesian coordinates $(-1.000 \text{ to } +1.000)$.
  - Vector magnitude $\sqrt{X^2 + Y^2}$ and polar travel angle ($0^\circ \text{ to } 360^\circ$).

### D. Stick Drift Detection
- **Status**: `REAL`
- **Methodology**:
  - User is prompted to release both thumbsticks.
  - Samples $N = 120$ consecutive frames of resting data.
  - Computes resting center offset, maximum deviation from $(128, 128)$, and Root-Mean-Square (RMS) jitter.
  - Compares deviation against configured deadzone threshold (default 8%).
  - Evaluates to `PASS`, `WARNING`, or `DRIFT DETECTED`.

### E. Range & Circularity Analysis
- **Status**: `REAL`
- **Methodology**:
  - Continuous bounding envelope tracking.
  - Computes effective axis range percentage: $\frac{Max - Min}{255} \times 100\%$.
  - Computes circularity symmetry error: $|Range_X - Range_Y|$.

### F. Software Calibration Wizard
- **Status**: `REAL (Software Profile Only)`
- **Clarification**: Due to hardware security, writes to DualShock 4 EEPROM are blocked by Sony. The application explicitly informs the user that calibration profiles are software profiles and saves them locally to `/data/RogueByte_Controller_Lab/profiles/calibration.json`.

### G. Dual-Motor Rumble Test
- **Status**: `REAL`
- **Methodology**: Calls `scePadSetVibration` passing distinct parameters to `largeMotor` (low-frequency heavy rumble) and `smallMotor` (high-frequency light rumble).
- **Controls**: Independent left motor, right motor, both motors, and adjustable intensity from 0% to 100%.

### H. Touchpad Capacitive Sensor
- **Status**: `REAL`
- **Methodology**: Reads `ScePadTouchData`.
- **Capabilities**: Tracks active finger contact, multi-touch presence, capacitive coordinate space ($1920 \times 941$), and physical click actuation.

### I. Diagnostic Reporting
- **Status**: `REAL`
- **Formats**: Formatted JSON and printable TXT reports exported directly to `/data/RogueByte_Controller_Lab/reports/`.
