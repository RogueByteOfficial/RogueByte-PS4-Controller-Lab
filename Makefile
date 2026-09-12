# ==============================================================================
# RogueByte PS4 Controller Lab - Native PS4 Homebrew Makefile
# Developer: RogueByte (Developed with ❤️ by Sido dev)
# Ko-fi: https://ko-fi.com/roguebyte
# GitHub: https://github.com/RogueByteOfficial
# ==============================================================================

CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -Isrc -std=c11
LDFLAGS ?= -lm

TITLE_ID := CUSA77123
APP_NAME := RogueByte PS4 Controller Lab
PKG_NAME := RogueByte_Controller_Lab.pkg

SRC := $(wildcard src/*.c) \
       $(wildcard src/controller/*.c) \
       $(wildcard src/hid/*.c) \
       $(wildcard src/joystick/*.c) \
       $(wildcard src/calibration/*.c) \
       $(wildcard src/vibration/*.c) \
       $(wildcard src/touchpad/*.c) \
       $(wildcard src/diagnostics/*.c) \
       $(wildcard src/storage/*.c) \
       $(wildcard src/reports/*.c) \
       $(wildcard src/ui/*.c)

OBJ := $(SRC:.c=.o)

.PHONY: all clean pkg test run

all: eboot.bin pkg

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

eboot.bin: $(OBJ)
	@echo "[+] Linking native PS4 application binary: eboot.bin..."
	$(CC) $(OBJ) $(LDFLAGS) -o eboot.bin
	@chmod +x eboot.bin

assets:
	@echo "[+] Verifying sce_sys PS4 assets and param.sfo..."
	@python3 tools/generate_assets.py
	@python3 tools/sfo_builder.py

pkg: eboot.bin assets
	@echo "[+] Building PS4 Homebrew Package (.pkg)..."
	@python3 tools/pkg_builder.py
	@echo "[SUCCESS] PKG generated at: dist/$(PKG_NAME)"

test:
	@echo "[+] Building and executing unit and regression test suite..."
	@$(CC) -Wall -Wextra -O2 -Isrc tests/test_suite.c \
		src/controller/controller.c \
		src/hid/hid_info.c \
		src/joystick/joystick.c \
		src/calibration/calibration.c \
		src/reports/reports.c \
		src/diagnostics/diagnostics.c \
		-lm -o tests/test_runner
	@./tests/test_runner

run-test-mode: eboot.bin
	@echo "[+] Running application in self-test frame validation mode..."
	@./eboot.bin --test-run

clean:
	@echo "[+] Cleaning build artifacts..."
	@rm -f $(OBJ) eboot.bin tests/test_runner dist/$(PKG_NAME) /tmp/test_profile.json
