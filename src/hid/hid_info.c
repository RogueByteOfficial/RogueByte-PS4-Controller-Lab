#include "hid_info.h"
#include "../controller/controller.h"
#include <stdio.h>
#include <string.h>

void hid_info_query(const void* ctrl_ptr, ControllerHidInfo* info) {
    if (!info) return;
    memset(info, 0, sizeof(ControllerHidInfo));
    
    const DS4Controller* ctrl = (const DS4Controller*)ctrl_ptr;
    
    if (!ctrl || !ctrl->connected) {
        strcpy(info->model, "NO CONTROLLER CONNECTED");
        strcpy(info->firmware, "NOT AVAILABLE");
        strcpy(info->vid_str, "NOT AVAILABLE");
        strcpy(info->pid_str, "NOT AVAILABLE");
        strcpy(info->serial, "NOT AVAILABLE");
        strcpy(info->connection, "DISCONNECTED");
        strcpy(info->battery, "NOT AVAILABLE");
        strcpy(info->usb_status, "DISCONNECTED");
        strcpy(info->bt_status, "DISCONNECTED");
        strcpy(info->unsupported_fields, "None (Device offline)");
        
        info->status_vid_pid = API_STATUS_FAILED;
        info->status_connection = API_STATUS_FAILED;
        info->status_battery = API_STATUS_NOT_SUPPORTED;
        info->status_firmware = API_STATUS_NOT_SUPPORTED;
        info->status_serial = API_STATUS_NOT_SUPPORTED;
        return;
    }
    
    /* Model Detection based on product ID */
    if (ctrl->product_id == 0x09CC) {
        strcpy(info->model, "DualShock 4 [CUH-ZCT2x - Gen 2 Slim/Pro]");
    } else {
        strcpy(info->model, "DualShock 4 [CUH-ZCT1x - Gen 1 Original]");
    }
    
    snprintf(info->vid_str, sizeof(info->vid_str), "0x%04X (Sony)", ctrl->vendor_id);
    snprintf(info->pid_str, sizeof(info->pid_str), "0x%04X", ctrl->product_id);
    info->status_vid_pid = API_STATUS_REAL;
    
    /* Connection type */
    if (strlen(ctrl->connection_type) > 0) {
        snprintf(info->connection, sizeof(info->connection), "%s", ctrl->connection_type);
        info->status_connection = API_STATUS_REAL;
        if (strcmp(ctrl->connection_type, "USB") == 0) {
            strcpy(info->usb_status, "Active (Wired Poll 250Hz)");
            strcpy(info->bt_status, "Standby");
        } else {
            strcpy(info->usb_status, "Inactive");
            strcpy(info->bt_status, "Active (Bluetooth L2CAP HID)");
        }
    } else {
        strcpy(info->connection, "NOT AVAILABLE");
        strcpy(info->usb_status, "NOT AVAILABLE");
        strcpy(info->bt_status, "NOT AVAILABLE");
        info->status_connection = API_STATUS_PARTIAL;
    }
    
    /* Battery Info */
    if (ctrl->battery_level >= 0) {
        snprintf(info->battery, sizeof(info->battery), "%d%%%s", 
                 ctrl->battery_level, ctrl->is_charging ? " (Charging)" : "");
        info->status_battery = API_STATUS_REAL;
    } else {
        strcpy(info->battery, "NOT AVAILABLE");
        info->status_battery = API_STATUS_NOT_SUPPORTED;
    }
    
    /* Firmware & Serial (PS4 user mode security sandbox restriction) */
    strcpy(info->firmware, "NOT AVAILABLE");
    info->status_firmware = API_STATUS_NOT_SUPPORTED;
    
    strcpy(info->serial, "NOT AVAILABLE");
    info->status_serial = API_STATUS_NOT_SUPPORTED;
    
    strcpy(info->unsupported_fields, 
           "Firmware Revision, EEPROM Factory Calib, Raw Serial Number\n"
           "(Requires Kernel / Syscon unauthenticated exploit)");
}
