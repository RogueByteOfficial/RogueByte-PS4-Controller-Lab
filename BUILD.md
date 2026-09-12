# Building RogueByte PS4 Controller Lab

This guide explains how to compile the native C code, generate PS4 system assets, and package the final `.pkg` file for installation on PlayStation 4 consoles.

---

## 1. System Requirements

### Host Environment
- **Operating System**: Linux (Ubuntu 20.04+, Debian 11+, Arch Linux, Fedora) or macOS / WSL2.
- **Compiler**: `gcc` (version 9+) or `clang` (version 10+).
- **Build Tools**: `make`, `build-essential`.
- **Python**: Python 3.8+ with `pillow` (`python3 -m pip install pillow`).

### Cross-Compilation (OpenOrbis Toolchain)
For targeting the native Orbis OS environment directly:
- **OpenOrbis PS4 Toolchain** (v0.5.2 or newer)
- Environment variable: `export OO_TOOLCHAIN=/path/to/OpenOrbis/toolchain`

---

## 2. Quick Build

From the root directory of the repository:

```bash
# Clean previous builds
make clean

# Compile native binary and package .pkg
make all

# Run unit tests
make test
```

Upon completion, you will find:
- Application executable: `./eboot.bin`
- Package file: `./dist/RogueByte_Controller_Lab.pkg`

---

## 3. Step-by-Step Build Pipeline

### Step 1: Asset Generation
PS4 Home Menu requires specific uncompressed/standard PNG files:
- `sce_sys/icon0.png`: 512×512 (App Icon)
- `sce_sys/pic0.png`: 1920×1080 (Launch Splash Banner)
- `sce_sys/pic1.png`: 1920×1080 (Background Wallpaper)

To regenerate these assets:
```bash
python3 tools/generate_assets.py
```

### Step 2: Param.sfo Creation
The `param.sfo` file contains critical system metadata:
- **Title ID**: `CUSA77123`
- **Application Type**: `gd` (Game Digital / Executable Homebrew)
- **Title Name**: `RogueByte PS4 Controller Lab`
- **Category**: Application
- **Version**: `01.00`
- **Target Firmware**: `05.050.000` (Compatible with 5.05 – 11.00)

To regenerate `param.sfo`:
```bash
python3 tools/sfo_builder.py
```

### Step 3: Compiling Native Source
The C11 codebase is organized under `src/`:
```bash
gcc -Wall -Wextra -O2 -Isrc -std=c11 \
    src/main.c \
    src/controller/controller.c \
    src/hid/hid_info.c \
    src/joystick/joystick.c \
    src/calibration/calibration.c \
    src/vibration/vibration.c \
    src/touchpad/touchpad.c \
    src/diagnostics/diagnostics.c \
    src/storage/storage.c \
    src/reports/reports.c \
    src/ui/font.c \
    src/ui/graphics.c \
    src/ui/ui.c \
    -lm -o eboot.bin
```

### Step 4: Packaging FPKG
The Python packaging tool packages the executable and system assets into an Orbis-compatible FPKG structure:
```bash
python3 tools/pkg_builder.py
```

---

## 4. Installing on PlayStation 4

1. Copy `dist/RogueByte_Controller_Lab.pkg` to the root of a **FAT32** or **exFAT** formatted USB flash drive.
2. Insert the USB flash drive into any USB port on your PS4 console.
3. Power on your PS4 running custom homebrew firmware (e.g. GoldHEN on 5.05, 6.72, 7.55, 9.00, or 11.00).
4. Navigate to:
   **Settings** ➔ **GoldHEN** (or **Debug Settings**) ➔ **Package Installer**.
5. Select `RogueByte_Controller_Lab.pkg` and press **Cross (✕)** to install.
6. Return to the PS4 Home Menu. The **RogueByte Controller Lab** icon will now appear in your main application list.
7. Launch directly from the Home Menu using your DualShock 4 controller.
