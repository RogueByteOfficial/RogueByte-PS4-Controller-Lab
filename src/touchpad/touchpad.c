#include "touchpad.h"
#include <string.h>

void touchpad_init(TouchpadState* ts) {
    if (!ts) return;
    memset(ts, 0, sizeof(TouchpadState));
    ts->api_status = API_STATUS_REAL;
    ts->test_result = RESULT_NOT_TESTED;
}

void touchpad_update(TouchpadState* ts, bool touch, bool click, uint16_t x, uint16_t y) {
    if (!ts) return;
    
    ts->touch_active = touch;
    ts->click_active = click;
    ts->current_x = x;
    ts->current_y = y;
    
    if (touch) {
        if (ts->trail_count < TOUCH_TRAIL_LEN) {
            ts->trail_x[ts->trail_count] = x;
            ts->trail_y[ts->trail_count] = y;
            ts->trail_count++;
        } else {
            /* Shift trail */
            memmove(&ts->trail_x[0], &ts->trail_x[1], (TOUCH_TRAIL_LEN - 1) * sizeof(uint16_t));
            memmove(&ts->trail_y[0], &ts->trail_y[1], (TOUCH_TRAIL_LEN - 1) * sizeof(uint16_t));
            ts->trail_x[TOUCH_TRAIL_LEN - 1] = x;
            ts->trail_y[TOUCH_TRAIL_LEN - 1] = y;
        }
        ts->test_result = RESULT_PASS;
    }
}

void touchpad_reset(TouchpadState* ts) {
    if (!ts) return;
    ts->trail_count = 0;
    ts->touch_active = false;
    ts->click_active = false;
    ts->test_result = RESULT_NOT_TESTED;
}
