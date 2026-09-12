# System Architecture

**RogueByte PS4 Controller Lab** is structured with a modular, layered architecture that strictly separates hardware abstraction, telemetry analysis, user interaction, and data persistence.

```
+-------------------------------------------------------------------------+
|                              Application Layer                          |
|                                 (main.c)                                |
+-------------------------------------------------------------------------+
       |                           |                         |
       v                           v                         v
+------------------+     +-------------------+     +---------------------+
|  UI & Rendering  |     |  Telemetry Math   |     | Hardware / HID      |
|  - graphics.c    |     |  - joystick.c     |     | - controller.c      |
|  - font.c        |     |  - calibration.c  |     | - hid_info.c        |
|  - ui.c          |     |  - diagnostics.c  |     | - touchpad.c        |
+------------------+     +-------------------+     | - vibration.c       |
                                                   +---------------------+
                                                             |
                                                             v
+------------------+     +-------------------+     +---------------------+
|  Data Storage    |     |  Diagnostic Report|     | PS4 Native Kernel   |
|  - storage.c     |     |  - reports.c      |     | - scePad            |
|  (/data/)        |     |  (JSON / TXT)     |     | - sceVideoOut       |
+------------------+     +-------------------+     +---------------------+
```

---

## 1. Architectural Layers

### A. Hardware & Controller Abstraction Layer (`src/controller/`, `src/hid/`)
- **Responsibility**: Interfaces with the Sony DualShock 4 controller.
- **PS4 Native Implementation**:
  - `scePadInit()` / `sceUserServiceGetInitialUser()`
  - `scePadOpen(userId, ORBIS_PAD_PORT_TYPE_STANDARD, 0, NULL)`
  - `scePadReadState(handle, &pad_data)`
  - `scePadSetVibration(handle, &vib)`
- **Disconnection Handling**: Tracks connection states frame-by-frame. If a controller disconnects, handles are freed safely without deadlocks or crashes, and reconnection attempts occur seamlessly.
- **Reality-Based Hardware Query (`src/hid/hid_info.c`)**:
  - Classifies readable parameters (`VID: 0x054C`, `PID: 0x05C4/0x09CC`, `USB Polling`).
  - Flags unexposed kernel parameters (`Firmware`, `EEPROM Serial Number`) as `NOT AVAILABLE` rather than returning simulated strings.

### B. Mathematical Analysis Engine (`src/joystick/`, `src/calibration/`)
- **Axis Normalization**:
  $$norm\_x = \frac{raw\_x - 128}{127.0}, \quad norm\_y = -\frac{raw\_y - 128}{127.0}$$
- **Drift Detection Engine**:
  - Collects $N = 120$ samples over 2 seconds at 60 Hz.
  - Computes resting center average:
    $$\bar{X} = \frac{1}{N}\sum_{i=1}^{N} X_i, \quad \bar{Y} = \frac{1}{N}\sum_{i=1}^{N} Y_i$$
  - Computes maximum deviation percentage:
    $$D_{max} = \frac{\max_i \sqrt{(X_i - 128)^2 + (Y_i - 128)^2}}{128.0} \times 100\%$$
  - Computes Root-Mean-Square (RMS) deviation percentage:
    $$RMS = \frac{\sqrt{\frac{1}{N}\sum_{i=1}^{N} ((X_i - \bar{X})^2 + (Y_i - \bar{Y})^2)}}{128.0} \times 100\%$$
  - Compares $D_{max}$ against user threshold (default 8.0%) to assert `PASS`, `WARNING`, or `DRIFT DETECTED`.
- **Circularity & Range Analysis**:
  - Records absolute minimum and maximum values along both Cartesian axes.
  - Checks bounding symmetry: $\Delta_{xy} = |Range_X - Range_Y|$.

### C. Native Graphics & UI Engine (`src/ui/`)
- **Pure Software Rendering**:
  - Operates on a 32-bit ARGB/RGBA linear buffer ($1920 \times 1080$, pitch 1920).
  - Implements sub-pixel Bresenham line rendering, filled/stroked rounded rectangles, circles, and anti-aliased crosshairs.
  - Direct raster glyph font pipeline ($8 \times 16$ font scaled $1\times, 2\times, 3\times$) avoiding any external FreeType or TrueType runtime overhead.
- **TV Safe Zones & Usability**:
  - Outer margins maintain a minimum 60px distance from display edges to prevent TV overscan clipping.
  - All text is scaled for comfortable legibility at a 10-foot couch distance.

### D. Diagnostics & Reporting Subsystem (`src/diagnostics/`, `src/reports/`)
- **Aggregator**: Collects test state from Button Matrix, Joystick Analyzer, Drift Engine, Touchpad, and Vibration.
- **Grading Matrix**:
  - `RESULT_PASS`
  - `RESULT_WARNING`
  - `RESULT_FAIL`
  - `RESULT_NOT_TESTED`
- **Exporters**:
  - Formatted JSON for automated parsing and tool interoperability.
  - ASCII formatted report card for technician viewing and documentation.

### E. Storage & Profile Layer (`src/storage/`)
- Persists data to the local PS4 partition under `/data/RogueByte_Controller_Lab/`.
- Maintains settings (`settings.bin`), export logs (`/reports/`), and user calibration profiles (`/profiles/`).
