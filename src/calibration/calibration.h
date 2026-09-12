#ifndef ROGUEBYTE_CALIBRATION_H
#define ROGUEBYTE_CALIBRATION_H

#include "../controller/types.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    CALIB_STEP_INTRO,         /* Step 1: Release both sticks */
    CALIB_STEP_LEFT_CENTER,   /* Step 2: Sample Left Center offset */
    CALIB_STEP_LEFT_RANGE,    /* Step 3: Move Left stick 360° */
    CALIB_STEP_RIGHT_CENTER,  /* Step 4: Sample Right Center offset */
    CALIB_STEP_RIGHT_RANGE,   /* Step 5: Move Right stick 360° */
    CALIB_STEP_RESULT         /* Step 6: Summary & Save Profile */
} CalibrationStep;

typedef struct {
    float center_x;
    float center_y;
    uint8_t min_x;
    uint8_t max_x;
    uint8_t min_y;
    uint8_t max_y;
    float deadzone;
} StickProfile;

typedef struct {
    char profile_name[64];
    char creation_date[32];
    bool is_software_only;    /* Strictly true: PS4 does not allow EEPROM write */
    
    StickProfile left;
    StickProfile right;
    
    /* Wizard state variables */
    CalibrationStep current_step;
    int sample_counter;
    double accum_x, accum_y;
} CalibrationWizard;

void calibration_init(CalibrationWizard* wiz);
void calibration_advance(CalibrationWizard* wiz);
void calibration_reset(CalibrationWizard* wiz);
void calibration_step_update(CalibrationWizard* wiz, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry);

/* Deadzone Mapping */
void calibration_apply_deadzone(float in_x, float in_y, float deadzone, float* out_x, float* out_y);

/* Persistence */
int calibration_save_profile(const CalibrationWizard* wiz, const char* filepath);
int calibration_load_profile(CalibrationWizard* wiz, const char* filepath);

#endif /* ROGUEBYTE_CALIBRATION_H */
