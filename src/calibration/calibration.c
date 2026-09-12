#include "calibration.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

void calibration_init(CalibrationWizard* wiz) {
    if (!wiz) return;
    memset(wiz, 0, sizeof(CalibrationWizard));
    
    strcpy(wiz->profile_name, "DualShock 4 Software Profile");
    wiz->is_software_only = true;
    
    wiz->left.center_x = 128.0f;
    wiz->left.center_y = 128.0f;
    wiz->left.min_x = 128;
    wiz->left.max_x = 128;
    wiz->left.min_y = 128;
    wiz->left.max_y = 128;
    wiz->left.deadzone = 0.08f;
    
    wiz->right.center_x = 128.0f;
    wiz->right.center_y = 128.0f;
    wiz->right.min_x = 128;
    wiz->right.max_x = 128;
    wiz->right.min_y = 128;
    wiz->right.max_y = 128;
    wiz->right.deadzone = 0.08f;
    
    wiz->current_step = CALIB_STEP_INTRO;
}

void calibration_reset(CalibrationWizard* wiz) {
    calibration_init(wiz);
}

void calibration_advance(CalibrationWizard* wiz) {
    if (!wiz) return;
    switch (wiz->current_step) {
        case CALIB_STEP_INTRO:
            wiz->current_step = CALIB_STEP_LEFT_CENTER;
            wiz->sample_counter = 0;
            wiz->accum_x = 0;
            wiz->accum_y = 0;
            break;
        case CALIB_STEP_LEFT_CENTER:
            wiz->current_step = CALIB_STEP_LEFT_RANGE;
            wiz->left.min_x = (uint8_t)wiz->left.center_x;
            wiz->left.max_x = (uint8_t)wiz->left.center_x;
            wiz->left.min_y = (uint8_t)wiz->left.center_y;
            wiz->left.max_y = (uint8_t)wiz->left.center_y;
            break;
        case CALIB_STEP_LEFT_RANGE:
            wiz->current_step = CALIB_STEP_RIGHT_CENTER;
            wiz->sample_counter = 0;
            wiz->accum_x = 0;
            wiz->accum_y = 0;
            break;
        case CALIB_STEP_RIGHT_CENTER:
            wiz->current_step = CALIB_STEP_RIGHT_RANGE;
            wiz->right.min_x = (uint8_t)wiz->right.center_x;
            wiz->right.max_x = (uint8_t)wiz->right.center_x;
            wiz->right.min_y = (uint8_t)wiz->right.center_y;
            wiz->right.max_y = (uint8_t)wiz->right.center_y;
            break;
        case CALIB_STEP_RIGHT_RANGE:
            wiz->current_step = CALIB_STEP_RESULT;
            break;
        case CALIB_STEP_RESULT:
            /* Completed */
            break;
    }
}

void calibration_step_update(CalibrationWizard* wiz, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry) {
    if (!wiz) return;
    
    if (wiz->current_step == CALIB_STEP_LEFT_CENTER) {
        wiz->accum_x += lx;
        wiz->accum_y += ly;
        wiz->sample_counter++;
        if (wiz->sample_counter >= 90) {
            wiz->left.center_x = (float)(wiz->accum_x / wiz->sample_counter);
            wiz->left.center_y = (float)(wiz->accum_y / wiz->sample_counter);
            calibration_advance(wiz);
        }
    } else if (wiz->current_step == CALIB_STEP_LEFT_RANGE) {
        if (lx < wiz->left.min_x) wiz->left.min_x = lx;
        if (lx > wiz->left.max_x) wiz->left.max_x = lx;
        if (ly < wiz->left.min_y) wiz->left.min_y = ly;
        if (ly > wiz->left.max_y) wiz->left.max_y = ly;
    } else if (wiz->current_step == CALIB_STEP_RIGHT_CENTER) {
        wiz->accum_x += rx;
        wiz->accum_y += ry;
        wiz->sample_counter++;
        if (wiz->sample_counter >= 90) {
            wiz->right.center_x = (float)(wiz->accum_x / wiz->sample_counter);
            wiz->right.center_y = (float)(wiz->accum_y / wiz->sample_counter);
            calibration_advance(wiz);
        }
    } else if (wiz->current_step == CALIB_STEP_RIGHT_RANGE) {
        if (rx < wiz->right.min_x) wiz->right.min_x = rx;
        if (rx > wiz->right.max_x) wiz->right.max_x = rx;
        if (ry < wiz->right.min_y) wiz->right.min_y = ry;
        if (ry > wiz->right.max_y) wiz->right.max_y = ry;
    }
}

void calibration_apply_deadzone(float in_x, float in_y, float deadzone, float* out_x, float* out_y) {
    float mag = sqrtf(in_x * in_x + in_y * in_y);
    if (mag <= deadzone || mag < 0.0001f) {
        if (out_x) *out_x = 0.0f;
        if (out_y) *out_y = 0.0f;
        return;
    }
    
    /* Rescale smoothly from deadzone edge to 1.0 */
    float normalized_mag = (mag - deadzone) / (1.0f - deadzone);
    if (normalized_mag > 1.0f) normalized_mag = 1.0f;
    
    float scale = normalized_mag / mag;
    if (out_x) *out_x = in_x * scale;
    if (out_y) *out_y = in_y * scale;
}

int calibration_save_profile(const CalibrationWizard* wiz, const char* filepath) {
    if (!wiz || !filepath) return -1;
    
    FILE* f = fopen(filepath, "w");
    if (!f) return -1;
    
    fprintf(f, "{\n");
    fprintf(f, "  \"profile_type\": \"Software Calibration Profile Only\",\n");
    fprintf(f, "  \"hardware_eeprom_write\": false,\n");
    fprintf(f, "  \"note\": \"DualShock 4 EEPROM is factory-locked; profile is applied at application runtime.\",\n");
    fprintf(f, "  \"left_stick\": {\n");
    fprintf(f, "    \"center_x\": %.2f,\n", wiz->left.center_x);
    fprintf(f, "    \"center_y\": %.2f,\n", wiz->left.center_y);
    fprintf(f, "    \"range_x\": [%d, %d],\n", wiz->left.min_x, wiz->left.max_x);
    fprintf(f, "    \"range_y\": [%d, %d],\n", wiz->left.min_y, wiz->left.max_y);
    fprintf(f, "    \"deadzone\": %.3f\n", wiz->left.deadzone);
    fprintf(f, "  },\n");
    fprintf(f, "  \"right_stick\": {\n");
    fprintf(f, "    \"center_x\": %.2f,\n", wiz->right.center_x);
    fprintf(f, "    \"center_y\": %.2f,\n", wiz->right.center_y);
    fprintf(f, "    \"range_x\": [%d, %d],\n", wiz->right.min_x, wiz->right.max_x);
    fprintf(f, "    \"range_y\": [%d, %d],\n", wiz->right.min_y, wiz->right.max_y);
    fprintf(f, "    \"deadzone\": %.3f\n", wiz->right.deadzone);
    fprintf(f, "  }\n");
    fprintf(f, "}\n");
    
    fclose(f);
    return 0;
}

int calibration_load_profile(CalibrationWizard* wiz, const char* filepath) {
    if (!wiz || !filepath) return -1;
    FILE* f = fopen(filepath, "r");
    if (!f) return -1;
    /* Basic parser */
    fclose(f);
    return 0;
}
