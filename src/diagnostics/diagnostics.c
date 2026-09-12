#include "diagnostics.h"
#include <string.h>

void diagnostics_init(DiagnosticSession* ds) {
    if (!ds) return;
    memset(ds, 0, sizeof(DiagnosticSession));
    
    ds->buttons_result = RESULT_NOT_TESTED;
    ds->left_stick_result = RESULT_NOT_TESTED;
    ds->right_stick_result = RESULT_NOT_TESTED;
    ds->left_drift_result = RESULT_NOT_TESTED;
    ds->right_drift_result = RESULT_NOT_TESTED;
    ds->touchpad_result = RESULT_NOT_TESTED;
    ds->vibration_result = RESULT_NOT_TESTED;
    ds->overall_result = RESULT_NOT_TESTED;
    
    ds->buttons_total_count = TOTAL_TESTABLE_BUTTONS;
    ds->current_step = SESSION_STEP_INACTIVE;
    ds->is_wizard_active = false;
}

void diagnostics_start_wizard(DiagnosticSession* ds) {
    if (!ds) return;
    diagnostics_init(ds);
    ds->is_wizard_active = true;
    ds->current_step = SESSION_STEP_DETECTION;
    ds->step_timer_frames = 0;
}

void diagnostics_advance_step(DiagnosticSession* ds) {
    if (!ds || !ds->is_wizard_active) return;
    
    switch (ds->current_step) {
        case SESSION_STEP_DETECTION:
            ds->current_step = SESSION_STEP_BUTTONS;
            break;
        case SESSION_STEP_BUTTONS:
            ds->current_step = SESSION_STEP_ANALOG;
            break;
        case SESSION_STEP_ANALOG:
            ds->current_step = SESSION_STEP_DRIFT;
            break;
        case SESSION_STEP_DRIFT:
            ds->current_step = SESSION_STEP_RANGE;
            break;
        case SESSION_STEP_RANGE:
            ds->current_step = SESSION_STEP_TOUCHPAD;
            break;
        case SESSION_STEP_TOUCHPAD:
            ds->current_step = SESSION_STEP_VIBRATION;
            break;
        case SESSION_STEP_VIBRATION:
            ds->current_step = SESSION_STEP_FINAL_REPORT;
            diagnostics_compute_overall(ds);
            break;
        case SESSION_STEP_FINAL_REPORT:
            ds->is_wizard_active = false;
            ds->current_step = SESSION_STEP_INACTIVE;
            break;
        default:
            ds->is_wizard_active = false;
            ds->current_step = SESSION_STEP_INACTIVE;
            break;
    }
}

void diagnostics_cancel_wizard(DiagnosticSession* ds) {
    if (!ds) return;
    ds->is_wizard_active = false;
    ds->current_step = SESSION_STEP_INACTIVE;
}

void diagnostics_compute_overall(DiagnosticSession* ds) {
    if (!ds) return;
    
    DiagnosticResult results[] = {
        ds->buttons_result,
        ds->left_stick_result,
        ds->right_stick_result,
        ds->left_drift_result,
        ds->right_drift_result,
        ds->touchpad_result,
        ds->vibration_result
    };
    
    int fail_count = 0;
    int warn_count = 0;
    int pass_count = 0;
    
    for (size_t i = 0; i < sizeof(results)/sizeof(results[0]); i++) {
        if (results[i] == RESULT_FAIL) fail_count++;
        else if (results[i] == RESULT_WARNING) warn_count++;
        else if (results[i] == RESULT_PASS) pass_count++;
    }
    
    if (fail_count > 0 || ds->left_drift_detected || ds->right_drift_detected) {
        ds->overall_result = RESULT_FAIL;
    } else if (warn_count > 0) {
        ds->overall_result = RESULT_WARNING;
    } else if (pass_count > 0) {
        ds->overall_result = RESULT_PASS;
    } else {
        ds->overall_result = RESULT_NOT_TESTED;
    }
}

const char* diagnostics_result_to_string(DiagnosticResult res) {
    switch (res) {
        case RESULT_PASS: return "PASS";
        case RESULT_WARNING: return "WARNING";
        case RESULT_FAIL: return "FAIL / DETECTED";
        case RESULT_NOT_TESTED: return "NOT TESTED";
        default: return "UNKNOWN";
    }
}
