# RogueByte PS4 Controller Lab

[![Platform](https://img.shields.io/badge/Platform-PS4%20Native%20Homebrew-blue.svg)](https://github.com/RogueByteOfficial/RogueByte-PS4-Controller-Lab)
[![Format](https://img.shields.io/badge/Format-PKG%20%28CUSA77123%29-brightgreen.svg)](https://github.com/RogueByteOfficial/RogueByte-PS4-Controller-Lab)
[![Tests](https://img.shields.io/badge/Tests-34%20Passed-emerald.svg)](https://github.com/RogueByteOfficial/RogueByte-PS4-Controller-Lab)
[![Support](https://img.shields.io/badge/Support-Ko--fi-ff5e5b.svg)](https://ko-fi.com/roguebyte)

**RogueByte PS4 Controller Lab** is a dedicated, fully native PlayStation 4 Homebrew application (`.pkg`) engineered specifically for hardware testing, stick drift diagnostic analysis, deadzone calibration profiling, and mechanical evaluation of Sony DualShock 4 controllers (CUH-ZCT1x / CUH-ZCT2x).

Built strictly in native C with hardware-level framebuffers and low-level PS4 gamepad interfaces. **Zero WebViews, zero HTML/JS, zero Electron, zero APK layers.**

---

## 🎮 Key Features & Capabilities

- **Direct Hardware HID Detection**:
  - Live discovery of DualShock 4 generation (Gen 1 vs Gen 2 Slim/Pro).
  - Accurate USB and Bluetooth polling state.
  - Transparent vendor & product IDs (`0x054C` / `0x05C4` or `0x09CC`).
  - Reality-based classification of sandbox-restricted fields (`Firmware`, `Serial No` marked as `NOT AVAILABLE` rather than falsified).

- **Complete 18-Button Real-Time Matrix**:
  - Independent testing of Cross (✕), Circle (◯), Square (□), Triangle (△).
  - D-Pad directional validation (Up, Down, Left, Right).
  - Shoulder switches (L1, R1) and clickable thumbsticks (L3, R3).
  - System buttons (Options, Share, PS Button, Touchpad Click).
  - 8-bit analog trigger pressure meters for L2 & R2 (0–255).
  - Interactive cumulative verification counter (18/18 PASS).

- **Precision Joystick & Stick Drift Diagnostic**:
  - Real-time reticles for Left Stick and Right Stick.
  - Live normalized axes $(-1.000 \text{ to } +1.000)$, polar magnitude, and travel angle.
  - Multi-sample mathematical drift test with resting center average $(\bar{X}, \bar{Y})$, Maximum Deviation %, and RMS Deviation %.
  - Visual deadzone boundary indicator (default 8%).

- **Joystick Range & Circularity Evaluation**:
  - Min/Max travel envelope recording.
  - Axis symmetry analysis and circularity error % detection.
  - Diagnostic classification (`PASS`, `WARNING`, `FAIL`).

- **Software Calibration Wizard**:
  - Multi-step interactive calibration workflow.
  - Generates reproducible Software Calibration Profiles (`/data/RogueByte_Controller_Lab/profiles/calibration.json`).
  - Strict honesty: Clarifies that hardware EEPROM recalibration is locked by Sony hardware, keeping adjustments safe and runtime-bound.

- **Dual-Motor Vibration Engine**:
  - Independent triggering of Heavy (Left Motor) and Light (Right Motor) eccentrics.
  - Synchronous dual-motor pulse testing with 0–100% intensity modulation.

- **Capacitive Touchpad Interface**:
  - Active coordinate tracking ($1920 \times 941$).
  - Multi-touch trail rendering with physical click detection.

- **Local Diagnostic Reporting**:
  - Instant export of hardware diagnostic records in standard JSON and human-readable TXT formats to `/data/RogueByte_Controller_Lab/reports/`.

---

## 📺 User Interface Architecture

- Designed for 1080p ($1920 \times 1080$) big-screen TV viewing at 10-foot distance.
- High-contrast Dark Onyx theme (`#0B0E14`) with Navy Slate cards and Brand Cyan (`#00D2FF`) accent lighting.
- Embedded raster font typography with zero runtime font dependencies.
- Fully controllable via D-Pad, Left Stick, Cross (Select), Circle (Back), and Options (Settings).

---

## 🛠️ Building & Packaging

### Prerequisites
- Standard Linux toolchain (`gcc` or `clang`, `make`, `python3`, `pillow`)
- Optional: OpenOrbis PS4 Toolchain (`OO_TOOLCHAIN`)

### Quick Build
```bash
# Build native binary and produce the .pkg file
make all

# Run automated logic and regression tests
make test

# Run native self-test validation mode (60 frames)
make run-test-mode
```

Output package location:
```
dist/RogueByte_Controller_Lab.pkg
```

For detailed compilation and OpenOrbis cross-compilation instructions, see [BUILD.md](BUILD.md).

---

## 📁 Repository Structure

```
├── assets/                  # High-resolution source icons and artwork
├── dist/                    # Compiled installable PS4 packages (.pkg)
├── sce_sys/                 # Mandatory PS4 system metadata & assets
│   ├── param.sfo            # SFO metadata (Title ID: CUSA77123)
│   ├── icon0.png            # 512x512 Home Menu Icon
│   ├── pic0.png             # 1920x1080 Launch Banner
│   └── pic1.png             # 1920x1080 Background Wallpaper
├── src/
│   ├── calibration/         # Software calibration wizard & deadzone math
│   ├── controller/          # DualShock 4 HID layer & button masks
│   ├── diagnostics/         # Full session wizard & grading algorithms
│   ├── hid/                 # Hardware metadata query & reality status
│   ├── joystick/            # Drift detection, RMS math & range evaluation
│   ├── reports/             # JSON and TXT report generators
│   ├── storage/             # Persistence for /data profiles & reports
│   ├── touchpad/            # DualShock 4 capacitive surface driver
│   ├── ui/                  # Native 1080p raster graphics & TV UI
│   ├── vibration/           # DualShock 4 rumble motor interface
│   └── main.c               # Main loop, video flip & event router
├── tests/
│   ├── test_suite.c         # Comprehensive automated test suite
│   └── test_runner          # Compiled native test runner binary
├── tools/
│   ├── generate_assets.py   # PS4 PNG asset generator
│   ├── sfo_builder.py       # Binary SFO generator
│   └── pkg_builder.py       # Standalone PS4 FPKG packager
├── Makefile                 # Automated build system
├── ARCHITECTURE.md          # System architecture and layer breakdown
├── BUILD.md                 # Complete compilation and toolchain guide
├── CAPABILITIES.md          # Feature matrix and Reality-Based status
├── LIMITATIONS.md           # Hardware and PS4 sandbox limitations
└── TESTING.md               # Automated testing and manual test protocols
```

---

## ⚖️ Reality-Based Engineering Matrix

| Feature Subsystem | Status | Technical Details |
| :--- | :---: | :--- |
| **Gamepad Detection** | `REAL` | Enumerates through PS4 `scePad` / native input handles |
| **18-Button Matrix** | `REAL` | Bitmask polling with edge-triggering and hold detection |
| **Analog Triggers (L2/R2)**| `REAL` | 8-bit precision (0–255) |
| **Drift Detection** | `REAL` | 120-frame mathematical sampling, RMS deviation, center offset |
| **Range & Circularity** | `REAL` | Envelope recording, axis symmetry delta, circularity % |
| **Touchpad Position** | `REAL` | $1920 \times 941$ capacitive coordinates and physical click |
| **Vibration Motors** | `REAL` | Dual independent motor controls via `ScePadVibrationParam` |
| **Local Reporting** | `REAL` | Saves JSON/TXT to `/data/RogueByte_Controller_Lab/` |
| **Software Calibration** | `REAL` | Software Profile generated and applied at runtime |
| **Firmware / Serial No** | `NOT AVAILABLE` | Strictly blocked by PS4 user sandbox; labeled transparently |
| **EEPROM Hardware Write** | `NOT AVAILABLE` | DualShock 4 EEPROM is factory OTP/write-protected |

---

## ❤️ Developer Support & Community

- **Developer**: RogueByte (Developed with ❤️ by **Sido dev**)
- **Ko-fi**: [https://ko-fi.com/roguebyte](https://ko-fi.com/roguebyte)
- **GitHub**: [https://github.com/RogueByteOfficial](https://github.com/RogueByteOfficial)

Dedicated to the PlayStation gaming, modding, and controller repair community.
