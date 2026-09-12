#ifndef ROGUEBYTE_HID_INFO_H
#define ROGUEBYTE_HID_INFO_H

#include "../controller/types.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char model[64];           /* "CUH-ZCT1 (Gen 1)" or "CUH-ZCT2 (Gen 2)" */
    char firmware[32];        /* "NOT AVAILABLE" (restricted in user mode) */
    char vid_str[16];         /* "0x054C" (Sony) */
    char pid_str[16];         /* "0x05C4" or "0x09CC" */
    char serial[32];          /* "NOT AVAILABLE" (hardware EEPROM protected) */
    char connection[16];      /* "USB" or "Bluetooth" */
    char battery[32];         /* "85% (Charging)" or "NOT AVAILABLE" */
    char usb_status[32];      /* "Connected / Active" */
    char bt_status[32];       /* "Standby / Not Paired" */
    char unsupported_fields[256]; /* Clearly listed unexposed hardware fields */
    
    ApiStatus status_vid_pid;
    ApiStatus status_connection;
    ApiStatus status_battery;
    ApiStatus status_firmware;
    ApiStatus status_serial;
} ControllerHidInfo;

void hid_info_query(const void* ctrl_ptr, ControllerHidInfo* info);

#endif /* ROGUEBYTE_HID_INFO_H */
