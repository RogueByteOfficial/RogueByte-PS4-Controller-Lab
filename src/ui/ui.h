#ifndef ROGUEBYTE_UI_H
#define ROGUEBYTE_UI_H

#include "graphics.h"
#include "../controller/controller.h"
#include "../joystick/joystick.h"
#include "../calibration/calibration.h"
#include "../vibration/vibration.h"
#include "../touchpad/touchpad.h"
#include "../diagnostics/diagnostics.h"
#include "../storage/storage.h"

typedef struct {
    AppView current_view;
    AppView previous_view;
    int menu_cursor;
    
    /* Sub-screens cursors / state */
    int deadzone_cursor;
    int vibration_cursor;
    int settings_cursor;
    
    /* Animation / Frame counter */
    uint32_t frame_count;
    
    /* Notification message banner */
    char toast_message[128];
    int toast_timer;
} UIState;

void ui_init(UIState* ui);
void ui_handle_input(UIState* ui, DS4Controller* ctrl, JoystickAnalyzer* ja, 
                     CalibrationWizard* calib, VibrationState* vib, 
                     TouchpadState* touch, DiagnosticSession* diag, 
                     AppSettings* settings);
void ui_render(UIState* ui, Framebuffer* fb, const DS4Controller* ctrl, 
               const JoystickAnalyzer* ja, const CalibrationWizard* calib, 
               const VibrationState* vib, const TouchpadState* touch, 
               const DiagnosticSession* diag, const AppSettings* settings);
void ui_show_toast(UIState* ui, const char* msg, int duration_frames);

#endif /* ROGUEBYTE_UI_H */
