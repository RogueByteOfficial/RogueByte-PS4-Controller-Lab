#include "joystick.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void joystick_normalize(uint8_t raw_x, uint8_t raw_y, float* norm_x, float* norm_y) {
    if (norm_x) {
        float nx = ((float)raw_x - 128.0f) / 127.0f;
        if (nx > 1.0f) nx = 1.0f;
        if (nx < -1.0f) nx = -1.0f;
        *norm_x = nx;
    }
    if (norm_y) {
        /* Invert Y so up is +1.0 and down is -1.0 */
        float ny = -(((float)raw_y - 128.0f) / 127.0f);
        if (ny > 1.0f) ny = 1.0f;
        if (ny < -1.0f) ny = -1.0f;
        *norm_y = ny;
    }
}

void joystick_analyzer_init(JoystickAnalyzer* ja) {
    if (!ja) return;
    memset(ja, 0, sizeof(JoystickAnalyzer));
    
    ja->user_deadzone_threshold = 0.08f; /* 8% threshold standard */
    
    joystick_reset_range_stats(&ja->left_stats);
    joystick_reset_range_stats(&ja->right_stats);
    
    ja->left_drift.deadzone_threshold = ja->user_deadzone_threshold;
    ja->right_drift.deadzone_threshold = ja->user_deadzone_threshold;
    
    ja->left_drift.state = DRIFT_STATE_IDLE;
    ja->right_drift.state = DRIFT_STATE_IDLE;
}

static void update_sample_stats(StickSample* sample, uint8_t rx, uint8_t ry) {
    sample->raw_x = rx;
    sample->raw_y = ry;
    joystick_normalize(rx, ry, &sample->norm_x, &sample->norm_y);
    sample->magnitude = sqrtf(sample->norm_x * sample->norm_x + sample->norm_y * sample->norm_y);
    
    float angle_rad = atan2f(sample->norm_y, sample->norm_x);
    float deg = angle_rad * (180.0f / (float)M_PI);
    if (deg < 0.0f) deg += 360.0f;
    sample->angle_deg = deg;
}

void joystick_analyzer_update(JoystickAnalyzer* ja, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry) {
    if (!ja) return;
    
    update_sample_stats(&ja->left_current, lx, ly);
    update_sample_stats(&ja->right_current, rx, ry);
    
    joystick_update_range_sample(&ja->left_stats, lx, ly);
    joystick_update_range_sample(&ja->right_stats, rx, ry);
}

void joystick_start_drift_test(JoystickAnalyzer* ja, float threshold) {
    if (!ja) return;
    if (threshold <= 0.0f) threshold = 0.08f;
    
    ja->left_drift.state = DRIFT_STATE_SAMPLING;
    ja->left_drift.sample_count = 0;
    ja->left_drift.max_samples = 120;
    ja->left_drift.deadzone_threshold = threshold;
    ja->left_drift.drift_detected = false;
    ja->left_drift.status = RESULT_NOT_TESTED;
    
    ja->right_drift.state = DRIFT_STATE_SAMPLING;
    ja->right_drift.sample_count = 0;
    ja->right_drift.max_samples = 120;
    ja->right_drift.deadzone_threshold = threshold;
    ja->right_drift.drift_detected = false;
    ja->right_drift.status = RESULT_NOT_TESTED;
}

bool joystick_drift_test_step(JoystickAnalyzer* ja, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry) {
    if (!ja) return true;
    
    if (ja->left_drift.state == DRIFT_STATE_SAMPLING) {
        int idx = ja->left_drift.sample_count;
        if (idx < DRIFT_MAX_SAMPLES) {
            ja->left_drift.samples_x[idx] = lx;
            ja->left_drift.samples_y[idx] = ly;
            ja->left_drift.sample_count++;
        }
        
        int r_idx = ja->right_drift.sample_count;
        if (r_idx < DRIFT_MAX_SAMPLES) {
            ja->right_drift.samples_x[r_idx] = rx;
            ja->right_drift.samples_y[r_idx] = ry;
            ja->right_drift.sample_count++;
        }
        
        if (ja->left_drift.sample_count >= ja->left_drift.max_samples &&
            ja->right_drift.sample_count >= ja->right_drift.max_samples) {
            
            joystick_finalize_drift_test(&ja->left_drift);
            joystick_finalize_drift_test(&ja->right_drift);
            return true; /* Complete */
        }
        return false; /* Still sampling */
    }
    return true;
}

void joystick_finalize_drift_test(DriftResult* res) {
    if (!res || res->sample_count == 0) return;
    
    int n = res->sample_count;
    double sum_x = 0.0, sum_y = 0.0;
    for (int i = 0; i < n; i++) {
        sum_x += res->samples_x[i];
        sum_y += res->samples_y[i];
    }
    res->center_avg_x = (float)(sum_x / n);
    res->center_avg_y = (float)(sum_y / n);
    
    float max_dist_sq = 0.0f;
    double sum_sq_diff = 0.0;
    
    for (int i = 0; i < n; i++) {
        float dx = (float)res->samples_x[i] - 128.0f;
        float dy = (float)res->samples_y[i] - 128.0f;
        float d_sq = dx * dx + dy * dy;
        if (d_sq > max_dist_sq) {
            max_dist_sq = d_sq;
        }
        
        float diff_x = (float)res->samples_x[i] - res->center_avg_x;
        float diff_y = (float)res->samples_y[i] - res->center_avg_y;
        sum_sq_diff += (diff_x * diff_x + diff_y * diff_y);
    }
    
    float max_dist = sqrtf(max_dist_sq);
    res->max_deviation_pct = (max_dist / 128.0f) * 100.0f;
    res->rms_deviation_pct = (sqrtf((float)(sum_sq_diff / n)) / 128.0f) * 100.0f;
    
    float thresh_pct = res->deadzone_threshold * 100.0f;
    
    if (res->max_deviation_pct > thresh_pct) {
        res->drift_detected = true;
        res->status = RESULT_FAIL;
    } else if (res->max_deviation_pct > (thresh_pct * 0.65f)) {
        res->drift_detected = false;
        res->status = RESULT_WARNING;
    } else {
        res->drift_detected = false;
        res->status = RESULT_PASS;
    }
    res->state = DRIFT_STATE_FINISHED;
}

void joystick_reset_range_stats(StickStats* stats) {
    if (!stats) return;
    stats->min_raw_x = 128;
    stats->max_raw_x = 128;
    stats->min_raw_y = 128;
    stats->max_raw_y = 128;
    stats->center_x = 128.0f;
    stats->center_y = 128.0f;
    stats->effective_range_x_pct = 0.0f;
    stats->effective_range_y_pct = 0.0f;
    stats->circularity_error_pct = 0.0f;
    stats->range_status = RESULT_NOT_TESTED;
}

void joystick_update_range_sample(StickStats* stats, uint8_t raw_x, uint8_t raw_y) {
    if (!stats) return;
    if (raw_x < stats->min_raw_x) stats->min_raw_x = raw_x;
    if (raw_x > stats->max_raw_x) stats->max_raw_x = raw_x;
    if (raw_y < stats->min_raw_y) stats->min_raw_y = raw_y;
    if (raw_y > stats->max_raw_y) stats->max_raw_y = raw_y;
}

void joystick_finalize_range_test(StickStats* stats) {
    if (!stats) return;
    
    float span_x = (float)(stats->max_raw_x - stats->min_raw_x);
    float span_y = (float)(stats->max_raw_y - stats->min_raw_y);
    
    stats->effective_range_x_pct = (span_x / 255.0f) * 100.0f;
    stats->effective_range_y_pct = (span_y / 255.0f) * 100.0f;
    
    /* Calculate circularity error based on axis symmetry */
    float diff_xy = fabsf(stats->effective_range_x_pct - stats->effective_range_y_pct);
    stats->circularity_error_pct = diff_xy;
    
    if (stats->effective_range_x_pct >= 90.0f && stats->effective_range_y_pct >= 90.0f && diff_xy <= 12.0f) {
        stats->range_status = RESULT_PASS;
    } else if (stats->effective_range_x_pct >= 75.0f && stats->effective_range_y_pct >= 75.0f) {
        stats->range_status = RESULT_WARNING;
    } else {
        stats->range_status = RESULT_FAIL;
    }
}
