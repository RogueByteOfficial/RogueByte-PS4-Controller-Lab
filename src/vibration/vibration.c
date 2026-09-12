#include "vibration.h"
#include <string.h>

#if defined(__ORBIS__) || defined(__PS4__)
#include <pad.h>

void vibration_init(VibrationState* vs) {
    if (!vs) return;
    memset(vs, 0, sizeof(VibrationState));
    vs->intensity_pct = 75;
    vs->api_status = API_STATUS_REAL;
}

int vibration_trigger(VibrationState* vs, int controller_handle, VibrationTarget target, uint8_t intensity_pct, int frames) {
    if (!vs || controller_handle <= 0) return -1;
    
    vs->target = target;
    vs->intensity_pct = intensity_pct;
    vs->duration_frames = frames > 0 ? frames : 60;
    vs->is_active = true;
    
    uint8_t raw_val = (uint8_t)((intensity_pct * 255) / 100);
    
    ScePadVibrationParam param;
    param.largeMotor = (target == VIB_TARGET_LEFT || target == VIB_TARGET_BOTH) ? raw_val : 0;
    param.smallMotor = (target == VIB_TARGET_RIGHT || target == VIB_TARGET_BOTH) ? raw_val : 0;
    
    int ret = scePadSetVibration(controller_handle, &param);
    if (ret != 0) {
        vs->api_status = API_STATUS_FAILED;
        return -1;
    }
    vs->api_status = API_STATUS_REAL;
    return 0;
}

void vibration_update(VibrationState* vs, int controller_handle) {
    if (!vs || !vs->is_active) return;
    
    if (vs->duration_frames > 0) {
        vs->duration_frames--;
        if (vs->duration_frames == 0) {
            vibration_stop(vs, controller_handle);
        }
    }
}

void vibration_stop(VibrationState* vs, int controller_handle) {
    if (!vs) return;
    vs->is_active = false;
    vs->duration_frames = 0;
    
    if (controller_handle > 0) {
        ScePadVibrationParam param;
        param.largeMotor = 0;
        param.smallMotor = 0;
        scePadSetVibration(controller_handle, &param);
    }
}

#else
/* Linux / Fallback */
void vibration_init(VibrationState* vs) {
    if (!vs) return;
    memset(vs, 0, sizeof(VibrationState));
    vs->intensity_pct = 75;
    vs->api_status = API_STATUS_PARTIAL;
}

int vibration_trigger(VibrationState* vs, int controller_handle, VibrationTarget target, uint8_t intensity_pct, int frames) {
    (void)controller_handle;
    if (!vs) return -1;
    vs->target = target;
    vs->intensity_pct = intensity_pct;
    vs->duration_frames = frames > 0 ? frames : 60;
    vs->is_active = true;
    return 0;
}

void vibration_update(VibrationState* vs, int controller_handle) {
    (void)controller_handle;
    if (!vs || !vs->is_active) return;
    if (vs->duration_frames > 0) {
        vs->duration_frames--;
        if (vs->duration_frames == 0) {
            vibration_stop(vs, controller_handle);
        }
    }
}

void vibration_stop(VibrationState* vs, int controller_handle) {
    (void)controller_handle;
    if (!vs) return;
    vs->is_active = false;
    vs->duration_frames = 0;
}
#endif
