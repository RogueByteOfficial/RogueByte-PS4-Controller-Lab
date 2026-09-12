#ifndef ROGUEBYTE_TYPES_H
#define ROGUEBYTE_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Reality-Based Engineering API Status */
typedef enum {
    API_STATUS_REAL,          /* Real hardware API call succeeded */
    API_STATUS_PARTIAL,       /* Real API with limited firmware access */
    API_STATUS_MOCK,          /* Mocked (strictly for offline testing) */
    API_STATUS_NOT_SUPPORTED, /* Feature not exposed by PS4 SDK */
    API_STATUS_FAILED         /* Hardware error or communication drop */
} ApiStatus;

/* DualShock 4 Button Bitmasks (PS4 Standard) */
#define DS4_BTN_L3         (1 << 1)   /* 0x0002 */
#define DS4_BTN_R3         (1 << 2)   /* 0x0004 */
#define DS4_BTN_OPTIONS    (1 << 3)   /* 0x0008 */
#define DS4_BTN_UP         (1 << 4)   /* 0x0010 */
#define DS4_BTN_RIGHT      (1 << 5)   /* 0x0020 */
#define DS4_BTN_DOWN       (1 << 6)   /* 0x0040 */
#define DS4_BTN_LEFT       (1 << 7)   /* 0x0080 */
#define DS4_BTN_L2         (1 << 8)   /* 0x0100 */
#define DS4_BTN_R2         (1 << 9)   /* 0x0200 */
#define DS4_BTN_L1         (1 << 10)  /* 0x0400 */
#define DS4_BTN_R1         (1 << 11)  /* 0x0800 */
#define DS4_BTN_TRIANGLE   (1 << 12)  /* 0x1000 */
#define DS4_BTN_CIRCLE     (1 << 13)  /* 0x2000 */
#define DS4_BTN_CROSS      (1 << 14)  /* 0x4000 */
#define DS4_BTN_SQUARE     (1 << 15)  /* 0x8000 */
#define DS4_BTN_SHARE      (1 << 16)  /* 0x10000 */
#define DS4_BTN_PS         (1 << 17)  /* 0x20000 */
#define DS4_BTN_TOUCHPAD   (1 << 18)  /* 0x40000 (Click) */

#define TOTAL_TESTABLE_BUTTONS 18

/* UI Application Views */
typedef enum {
    VIEW_MAIN_MENU,
    VIEW_CONTROLLER_TEST,
    VIEW_JOYSTICK_ANALYZER,
    VIEW_DRIFT_DETECTOR,
    VIEW_RANGE_TEST,
    VIEW_CALIBRATION_WIZARD,
    VIEW_DEADZONE_ANALYZER,
    VIEW_VIBRATION_TEST,
    VIEW_TOUCHPAD_TEST,
    VIEW_CONTROLLER_INFO,
    VIEW_DIAGNOSTIC_REPORT,
    VIEW_FULL_DIAGNOSTIC_SESSION,
    VIEW_SETTINGS,
    VIEW_ABOUT_DEV,
    VIEW_DISCONNECTED_DIALOG
} AppView;

/* Diagnostic Result Flags */
typedef enum {
    RESULT_NOT_TESTED,
    RESULT_PASS,
    RESULT_WARNING,
    RESULT_FAIL
} DiagnosticResult;

#endif /* ROGUEBYTE_TYPES_H */
