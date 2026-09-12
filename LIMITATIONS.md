# Technical Boundaries & Limitations

In accordance with our **Reality-Based Engineering** ethos, this document transparently documents the technical, architectural, and security boundaries that apply to **RogueByte PS4 Controller Lab**.

---

## 1. PlayStation 4 User Sandbox Restrictions

PlayStation 4 Homebrew applications execute within unprivileged user space. The following hardware-level queries are restricted by the Orbis OS kernel and are marked as `NOT AVAILABLE` in the user interface:

### A. Controller Firmware Version
- **Restriction**: Sony does not expose the DualShock 4 microcontroller firmware revision (e.g. `01.03`, `01.07`) via standard `scePad` user-mode APIs.
- **Why**: Firmware version interrogation requires issuing raw vendor-specific USB control transfers (`0xA2 / 0x02` GET_REPORT) through raw kernel handles, which are restricted.
- **Handling**: Explicitly displayed as `NOT AVAILABLE (Protected by PS4 user sandbox)`.

### B. Hardware Serial Number
- **Restriction**: DualShock 4 printed serial numbers are stored inside the hardware EEPROM alongside factory calibration matrices.
- **Why**: Direct I2C/SPI access to the internal gamepad EEPROM is blocked from unprivileged applications.
- **Handling**: Explicitly displayed as `NOT AVAILABLE (Factory locked EEPROM)`.

---

## 2. Calibration & EEPROM Write Restrictions

### Hardware EEPROM Calibration vs Software Calibration
- **Factory Calibration**: Genuine Sony controllers contain factory-programmed neutral offsets inside their onboard EEPROM. Once an analog stick develops mechanical wear (such as resistive wiper degradation or spring slack), adjusting this offset permanently requires either:
  1. Desoldering and installing Hall-Effect sensors.
  2. Physical recalibration with specialized hardware jigs.
  3. Writing to EEPROM using an external hardware programmer (such as an ST-Link or custom Teensy flasher).
- **Application Policy**: **RogueByte Controller Lab will never falsely claim to write or "fix" hardware EEPROM calibration.**
- **Implementation**: The application generates a **Software Calibration Profile** saved locally to `/data/RogueByte_Controller_Lab/profiles/calibration.json`. This profile defines software-level deadzones and center corrections for game engines or runtime wrappers.

---

## 3. Third-Party Controller Compatibility

| Controller Brand / Model | Buttons & Sticks | Touchpad Tracking | Vibration Test |
| :--- | :---: | :---: | :---: |
| **Sony DualShock 4 (CUH-ZCT1x)** | Fully Supported | Fully Supported | Fully Supported |
| **Sony DualShock 4 (CUH-ZCT2x)** | Fully Supported | Fully Supported | Fully Supported |
| **Scuf Infinity4PS / Impact** | Fully Supported | Fully Supported | Fully Supported |
| **Razer Raiju / Raiju Ultimate** | Fully Supported | Fully Supported | Fully Supported |
| **Nacon Revolution Pro (v1/v2/v3)** | Fully Supported | Supported | Supported |
| **Generic Third-Party DS4 Clones** | Supported | Basic / May lack multi-touch | May lack dual-motor nuance |
| **DualSense (PS5) on PS4** | Only via USB adapter / remote play | Partial | May not rumble natively |

---

## 4. Hardware Disconnection Resilience

If the controller is physically unplugged or the battery runs out during an active test:
- The input poller catches the disconnection event immediately.
- Active motor vibration commands are stopped to prevent motor burnout.
- The UI safely displays a "NO CONTROLLER DETECTED" banner without locking or crashing the render loop.
- Reconnection is automatically detected when the gamepad is reattached.
