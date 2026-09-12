#ifndef ROGUEBYTE_JOYSTICK_H
#define ROGUEBYTE_JOYSTICK_H

#include "../controller/types.h"
#include <stdint.h>
#include <stdbool.h>

#define DRIFT_MAX_SAMPLES 180

typedef struct {
    uint8_t raw_x;
    uint8_t raw_y;
    float norm_x;      /* -1.000 to +1.000 */
    float norm_y;      /* -1.000 to +1.000 */
    float magnitude;   /* 0.000 to 1.414 */
    float angle_deg;   /* 0.0 to 360.0 */
} StickSample;

typedef struct {
    /* Extreme limits observed */
    uint8_t min_raw_x;
    uint8_t max_raw_x;
    uint8_t min_raw_y;
    uint8_t max_raw_y;
    
    /* Center resting values */
    float center_x;
    float center_y;
    
    /* Range stats */
    float effective_range_x_pct;
    float effective_range_y_pct;
    float circularity_error_pct;
    
    DiagnosticResult range_status;
} StickStats;

typedef enum {
    DRIFT_STATE_IDLE,
    DRIFT_STATE_COUNTDOWN,
    DRIFT_STATE_SAMPLING,
    DRIFT_STATE_FINISHED
} DriftTestState;

typedef struct {
    DriftTestState state;
    int sample_count;
    int max_samples;
    uint8_t samples_x[DRIFT_MAX_SAMPLES];
    uint8_t samples_y[DRIFT_MAX_SAMPLES];
    
    float center_avg_x;
    float center_avg_y;
    float max_deviation_pct;
    float rms_deviation_pct;
    float deadzone_threshold; /* e.g. 0.08 (8%) */
    
    bool drift_detected;
    DiagnosticResult status;
} DriftResult;

typedef struct {
    StickSample left_current;
    StickSample right_current;
    StickStats  left_stats;
    StickStats  right_stats;
    DriftResult left_drift;
    DriftResult right_drift;
    float user_deadzone_threshold; /* default 0.08 */
} JoystickAnalyzer;

/* Normalization */
void joystick_normalize(uint8_t raw_x, uint8_t raw_y, float* norm_x, float* norm_y);

/* Lifecycle & Analysis */
void joystick_analyzer_init(JoystickAnalyzer* ja);
void joystick_analyzer_update(JoystickAnalyzer* ja, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry);

/* Drift Testing Engine */
void joystick_start_drift_test(JoystickAnalyzer* ja, float threshold);
bool joystick_drift_test_step(JoystickAnalyzer* ja, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry);
void joystick_finalize_drift_test(DriftResult* res);

/* Range & Circularity */
void joystick_reset_range_stats(StickStats* stats);
void joystick_update_range_sample(StickStats* stats, uint8_t raw_x, uint8_t raw_y);
void joystick_finalize_range_test(StickStats* stats);

#endif /* ROGUEBYTE_JOYSTICK_H */
