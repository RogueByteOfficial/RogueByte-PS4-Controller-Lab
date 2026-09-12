#ifndef ROGUEBYTE_TOUCHPAD_H
#define ROGUEBYTE_TOUCHPAD_H

#include "../controller/types.h"
#include <stdint.h>
#include <stdbool.h>

#define TOUCHPAD_WIDTH  1920
#define TOUCHPAD_HEIGHT  941
#define TOUCH_TRAIL_LEN   32

typedef struct {
    uint16_t x;
    uint16_t y;
    bool active;
} TouchPoint;

typedef struct {
    bool touch_active;
    bool click_active;
    uint16_t current_x;
    uint16_t current_y;
    uint8_t touch_count;
    
    TouchPoint finger[2];
    
    /* Trail buffer for drawing motion on screen */
    uint16_t trail_x[TOUCH_TRAIL_LEN];
    uint16_t trail_y[TOUCH_TRAIL_LEN];
    int trail_count;
    
    ApiStatus api_status;
    DiagnosticResult test_result;
} TouchpadState;

void touchpad_init(TouchpadState* ts);
void touchpad_update(TouchpadState* ts, bool touch, bool click, uint16_t x, uint16_t y);
void touchpad_reset(TouchpadState* ts);

#endif /* ROGUEBYTE_TOUCHPAD_H */
