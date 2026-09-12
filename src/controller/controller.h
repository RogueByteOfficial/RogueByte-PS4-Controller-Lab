#ifndef ROGUEBYTE_CONTROLLER_H
#define ROGUEBYTE_CONTROLLER_H

#include "types.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool connected;
    int handle;
    int32_t user_id;
    char connection_type[16];   /* "USB", "Bluetooth", "NOT AVAILABLE" */
    char device_name[64];       /* "DualShock 4 Wireless Controller" */
    uint16_t vendor_id;         /* 0x054C (Sony Interactive Entertainment) */
    uint16_t product_id;        /* 0x05C4 (v1) or 0x09CC (v2) */
    
    /* Digital & Analog Inputs */
    uint32_t buttons;           /* Bitmask of active buttons */
    uint32_t buttons_prev;      /* Previous frame for edge-trigger */
    uint32_t buttons_tested;    /* Bitmask of buttons pressed during test */
    
    uint8_t lx, ly;             /* Left Stick raw (0-255, center ~128) */
    uint8_t rx, ry;             /* Right Stick raw (0-255, center ~128) */
    uint8_t l2_analog;          /* L2 analog trigger (0-255) */
    uint8_t r2_analog;          /* R2 analog trigger (0-255) */
    
    /* Touchpad state */
    bool touch_active;
    bool touch_clicked;
    uint16_t touch_x;           /* 0 - 1919 */
    uint16_t touch_y;           /* 0 - 941 */
    uint8_t touch_id;
    uint8_t touch_count;
    
    /* Battery & Status */
    int8_t battery_level;       /* -1 if NOT AVAILABLE, 0-100 */
    bool is_charging;
    
    /* Reality classification */
    ApiStatus status_detection;
    ApiStatus status_buttons;
    ApiStatus status_analog;
    ApiStatus status_vibration;
    ApiStatus status_touchpad;
} DS4Controller;

/* Lifecycle */
int controller_init(DS4Controller* ctrl);
int controller_poll(DS4Controller* ctrl);
void controller_close(DS4Controller* ctrl);

/* Helpers */
bool controller_button_pressed(const DS4Controller* ctrl, uint32_t btn);
bool controller_button_down(const DS4Controller* ctrl, uint32_t btn);
bool controller_button_released(const DS4Controller* ctrl, uint32_t btn);

/* Test Reset */
void controller_reset_tested_buttons(DS4Controller* ctrl);
int controller_get_tested_button_count(const DS4Controller* ctrl);

#endif /* ROGUEBYTE_CONTROLLER_H */
