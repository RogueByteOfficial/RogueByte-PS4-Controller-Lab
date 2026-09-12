# Testing & Quality Assurance

This document details the automated test suite, regression procedures, and manual verification protocols used to ensure the reliability and mathematical accuracy of **RogueByte PS4 Controller Lab**.

---

## 1. Automated Test Suite (`tests/test_suite.c`)

The automated test runner validates all mathematical algorithms, state machines, and data formatting pipelines independently of physical hardware:

```bash
make test
```

### Test Coverage Overview

| Test Module | Test Name | Invariants Verified |
| :--- | :--- | :--- |
| **Normalization** | `test_normalization` | Center $(128, 128) \to (0.0, 0.0)$, Extremes $(0, 255) \to (-1.0, +1.0)$ |
| **Deadzone Math** | `test_deadzone_math` | Clamps within threshold, smoothly rescales outside $(D_{zone} \to 1.0)$ |
| **Drift Engine** | `test_drift_math` | Ideal centered stick evaluates to `PASS`; Offset stick evaluates to `DRIFT DETECTED` |
| **Circularity** | `test_range_circularity`| Travel envelope $>90\%$, circularity error $<5\%$ |
| **Calibration** | `test_calibration_profile`| Strict software profile tagging, JSON serialization integrity |
| **Reporting** | `test_report_generation`| Generates conforming JSON schema & formatted ASCII diagnostic card |
| **Disconnection** | `test_controller_disconnection`| Graceful fallback to `NOT AVAILABLE` without memory faults |

**Result**: 34 Test Cases, **0 Failures**, **0 Regressions**.

---

## 2. Standalone Frame Validation Mode

To verify video rendering, event routing, and frame timing on host machines without an Orbis kernel:

```bash
make run-test-mode
```

This runs the full application loop through 60 frames of rendering, simulating real gameplay loops and certifying that no memory leaks or segmentation faults occur.

---

## 3. Manual Physical Verification Protocol (PlayStation 4)

Follow this checklist when testing on a physical PlayStation 4 console:

### Phase 1: Installation & Launch
- [ ] Transfer `RogueByte_Controller_Lab.pkg` via USB.
- [ ] Install via GoldHEN / Debug Package Installer.
- [ ] Confirm the custom icon (`icon0.png`) and title appear in the PS4 Home Menu.
- [ ] Launch application; verify that the 1080p UI renders without overscan clipping.

### Phase 2: Input & Hardware Verification
- [ ] **Controller Test**: Press all 18 buttons individually. Verify each button illuminates green when pressed and increments the counter to `18 / 18`.
- [ ] **Analog Triggers**: Slowly depress L2 and R2; observe smooth 0–255 progress bar filling.
- [ ] **Joystick Analyzer**: Move left and right sticks in circles; observe smooth reticle tracking.
- [ ] **Drift Detection**: Set controller on table and run the test; confirm that resting deviation is measured accurately.
- [ ] **Touchpad Test**: Slide finger across surface; verify coordinates update from $(0, 0)$ to $(1920, 941)$ and physical click is detected.
- [ ] **Vibration Test**: Trigger Left Motor, Right Motor, and Both; verify that rumble stops cleanly.
- [ ] **Report Export**: Press Cross on Diagnostic Report screen; confirm files are created under `/data/RogueByte_Controller_Lab/reports/`.

### Phase 3: Stress & Disconnection
- [ ] Disconnect USB cable during an active test; verify the UI shows "NO CONTROLLER DETECTED" without hanging.
- [ ] Reconnect USB cable; verify the controller re-enumerates immediately.
