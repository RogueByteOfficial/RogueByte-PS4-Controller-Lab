#ifndef ROGUEBYTE_DIAGNOSTICS_H
#define ROGUEBYTE_DIAGNOSTICS_H

#include "../controller/types.h"
#include <stdbool.h>

typedef enum {
    SESSION_STEP_DETECTION,
    SESSION_STEP_BUTTONS,
    SESSION_STEP_ANALOG,
    SESSION_STEP_DRIFT,
    SESSION_STEP_RANGE,
    SESSION_STEP_TOUCHPAD,
    SESSION_STEP_VIBRATION,
    SESSION_STEP_FINAL_REPORT,
    SESSION_STEP_INACTIVE
} DiagnosticSessionStep;

typedef struct {
    DiagnosticResult buttons_result;
    DiagnosticResult left_stick_result;
    DiagnosticResult right_stick_result;
    DiagnosticResult left_drift_result;
    DiagnosticResult right_drift_result;
    DiagnosticResult touchpad_result;
    DiagnosticResult vibration_result;
    DiagnosticResult overall_result;
    
    int buttons_passed_count;
    int buttons_total_count;
    
    float left_drift_deviation_pct;
    float right_drift_deviation_pct;
    bool left_drift_detected;
    bool right_drift_detected;
    
    float left_range_pct;
    float right_range_pct;
    
    /* Wizard Session Control */
    bool is_wizard_active;
    DiagnosticSessionStep current_step;
    int step_timer_frames;
} DiagnosticSession;

void diagnostics_init(DiagnosticSession* ds);
void diagnostics_start_wizard(DiagnosticSession* ds);
void diagnostics_advance_step(DiagnosticSession* ds);
void diagnostics_cancel_wizard(DiagnosticSession* ds);
void diagnostics_compute_overall(DiagnosticSession* ds);

const char* diagnostics_result_to_string(DiagnosticResult res);

#endif /* ROGUEBYTE_DIAGNOSTICS_H */
