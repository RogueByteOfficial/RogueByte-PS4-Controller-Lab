#ifndef ROGUEBYTE_VIBRATION_H
#define ROGUEBYTE_VIBRATION_H

#include "../controller/types.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    VIB_TARGET_NONE,
    VIB_TARGET_LEFT,
    VIB_TARGET_RIGHT,
    VIB_TARGET_BOTH
} VibrationTarget;

typedef struct {
    VibrationTarget target;
    uint8_t intensity_pct;  /* 0 - 100% */
    bool is_active;
    int duration_frames;
    ApiStatus api_status;
} VibrationState;

void vibration_init(VibrationState* vs);
int vibration_trigger(VibrationState* vs, int controller_handle, VibrationTarget target, uint8_t intensity_pct, int frames);
void vibration_update(VibrationState* vs, int controller_handle);
void vibration_stop(VibrationState* vs, int controller_handle);

#endif /* ROGUEBYTE_VIBRATION_H */
