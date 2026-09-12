#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "../src/controller/controller.h"
#include "../src/hid/hid_info.h"
#include "../src/joystick/joystick.h"
#include "../src/calibration/calibration.h"
#include "../src/reports/reports.h"
#include "../src/diagnostics/diagnostics.h"

static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define ASSERT_TEST(cond, msg) do { \
    if (cond) { \
        printf("  [PASS] %s\n", msg); \
        g_tests_passed++; \
    } else { \
        printf("  [FAIL] %s\n", msg); \
        g_tests_failed++; \
    } \
} while(0)

/* 1. Test Normalization */
void test_normalization(void) {
    printf("\n=== TEST: Joystick Normalization ===\n");
    float nx, ny;
    
    /* Center (128, 128) */
    joystick_normalize(128, 128, &nx, &ny);
    ASSERT_TEST(fabsf(nx) < 0.01f, "Center X maps to ~0.0");
    ASSERT_TEST(fabsf(ny) < 0.01f, "Center Y maps to ~0.0");
    
    /* Full Right (255, 128) */
    joystick_normalize(255, 128, &nx, &ny);
    ASSERT_TEST(fabsf(nx - 1.0f) < 0.01f, "Max Right X maps to +1.0");
    
    /* Full Left (0, 128) */
    joystick_normalize(0, 128, &nx, &ny);
    ASSERT_TEST(fabsf(nx - (-1.0f)) < 0.01f, "Max Left X maps to -1.0");
    
    /* Full Up (128, 0) -> Up is positive in our inverted coordinates */
    joystick_normalize(128, 0, &nx, &ny);
    ASSERT_TEST(fabsf(ny - 1.0f) < 0.01f, "Max Up Y maps to +1.0");
    
    /* Full Down (128, 255) */
    joystick_normalize(128, 255, &nx, &ny);
    ASSERT_TEST(fabsf(ny - (-1.0f)) < 0.01f, "Max Down Y maps to -1.0");
}

/* 2. Test Deadzone Calculation */
void test_deadzone_math(void) {
    printf("\n=== TEST: Deadzone Math & Rescaling ===\n");
    float out_x, out_y;
    float deadzone = 0.10f; /* 10% */
    
    /* Input inside deadzone (0.05, 0.05) -> magnitude ~0.07 <= 0.10 */
    calibration_apply_deadzone(0.05f, 0.05f, deadzone, &out_x, &out_y);
    ASSERT_TEST(out_x == 0.0f && out_y == 0.0f, "Inputs inside deadzone mapped to zero");
    
    /* Input outside deadzone (0.50, 0.00) */
    calibration_apply_deadzone(0.50f, 0.00f, deadzone, &out_x, &out_y);
    ASSERT_TEST(out_x > 0.0f, "Inputs outside deadzone smoothly passed");
    ASSERT_TEST(fabsf(out_x - (0.5f - 0.1f)/(1.0f - 0.1f)) < 0.01f, "Deadzone properly rescaled (0.444)");
}

/* 3. Test Drift Detection Math */
void test_drift_math(void) {
    printf("\n=== TEST: Drift Detection Engine ===\n");
    DriftResult res;
    memset(&res, 0, sizeof(res));
    res.sample_count = 100;
    res.deadzone_threshold = 0.08f; /* 8% */
    
    /* Scenario A: Perfect Centered Controller (no drift) */
    for (int i = 0; i < 100; i++) {
        res.samples_x[i] = 128;
        res.samples_y[i] = 128;
    }
    joystick_finalize_drift_test(&res);
    ASSERT_TEST(res.drift_detected == false, "Ideal centered stick: NO drift detected");
    ASSERT_TEST(res.status == RESULT_PASS, "Ideal centered stick: PASS result");
    ASSERT_TEST(res.max_deviation_pct < 0.01f, "Ideal stick has 0.00% deviation");
    
    /* Scenario B: Drifting stick (resting at X=148, Y=145 -> ~18% deviation) */
    memset(&res, 0, sizeof(res));
    res.sample_count = 100;
    res.deadzone_threshold = 0.08f;
    for (int i = 0; i < 100; i++) {
        res.samples_x[i] = 148;
        res.samples_y[i] = 145;
    }
    joystick_finalize_drift_test(&res);
    ASSERT_TEST(res.drift_detected == true, "Offset stick: DRIFT DETECTED");
    ASSERT_TEST(res.status == RESULT_FAIL, "Offset stick: FAIL result");
    ASSERT_TEST(res.max_deviation_pct > 15.0f, "Offset stick: deviation exceeds threshold");
}

/* 4. Test Range & Circularity Math */
void test_range_circularity(void) {
    printf("\n=== TEST: Joystick Range & Circularity ===\n");
    StickStats stats;
    joystick_reset_range_stats(&stats);
    
    /* Simulate full circular travel */
    stats.min_raw_x = 5;
    stats.max_raw_x = 250;
    stats.min_raw_y = 6;
    stats.max_raw_y = 251;
    
    joystick_finalize_range_test(&stats);
    ASSERT_TEST(stats.effective_range_x_pct > 90.0f, "Effective Range X exceeds 90%");
    ASSERT_TEST(stats.effective_range_y_pct > 90.0f, "Effective Range Y exceeds 90%");
    ASSERT_TEST(stats.circularity_error_pct < 5.0f, "Circularity error within 5% tolerance");
    ASSERT_TEST(stats.range_status == RESULT_PASS, "Full range test evaluates to PASS");
}

/* 5. Test Calibration Profile Generation */
void test_calibration_profile(void) {
    printf("\n=== TEST: Calibration Software Profile ===\n");
    CalibrationWizard wiz;
    calibration_init(&wiz);
    
    wiz.left.center_x = 127.4f;
    wiz.left.center_y = 129.1f;
    wiz.left.min_x = 0;
    wiz.left.max_x = 255;
    wiz.left.deadzone = 0.08f;
    
    ASSERT_TEST(wiz.is_software_only == true, "Profile strictly flagged as Software Profile Only");
    
    int save_res = calibration_save_profile(&wiz, "/tmp/test_profile.json");
    ASSERT_TEST(save_res == 0, "Saved profile JSON successfully to disk");
    
    FILE* f = fopen("/tmp/test_profile.json", "r");
    ASSERT_TEST(f != NULL, "Profile JSON file exists on disk");
    if (f) {
        char buf[1024];
        size_t n = fread(buf, 1, sizeof(buf) - 1, f);
        buf[n] = '\0';
        fclose(f);
        ASSERT_TEST(strstr(buf, "Software Calibration Profile Only") != NULL, "Profile JSON explicitly contains software-only notice");
        ASSERT_TEST(strstr(buf, "hardware_eeprom_write") != NULL, "Profile JSON contains eeprom status");
    }
}

/* 6. Test Report Generation */
void test_report_generation(void) {
    printf("\n=== TEST: Report Generation (JSON & TXT) ===\n");
    DS4Controller ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.connected = true;
    strcpy(ctrl.device_name, "DualShock 4 Wireless Controller");
    strcpy(ctrl.connection_type, "USB");
    ctrl.vendor_id = 0x054C;
    ctrl.product_id = 0x05C4;
    
    DiagnosticSession diag;
    diagnostics_init(&diag);
    diag.buttons_result = RESULT_PASS;
    diag.buttons_passed_count = 18;
    diag.buttons_total_count = 18;
    diag.left_stick_result = RESULT_PASS;
    diag.right_stick_result = RESULT_PASS;
    diag.left_drift_result = RESULT_PASS;
    diag.right_drift_result = RESULT_PASS;
    diag.overall_result = RESULT_PASS;
    
    char json_buf[2048];
    char txt_buf[2048];
    
    int j_len = report_generate_json(&ctrl, &diag, json_buf, sizeof(json_buf));
    int t_len = report_generate_text(&ctrl, &diag, txt_buf, sizeof(txt_buf));
    
    ASSERT_TEST(j_len > 0, "Generated JSON report with valid length");
    ASSERT_TEST(strstr(json_buf, "RogueByte PS4 Controller Lab") != NULL, "JSON contains application header");
    ASSERT_TEST(strstr(json_buf, "\"buttons\": \"PASS\"") != NULL, "JSON contains button test pass");
    
    ASSERT_TEST(t_len > 0, "Generated TXT report with valid length");
    ASSERT_TEST(strstr(txt_buf, "ROGUEBYTE PS4 CONTROLLER LAB") != NULL, "TXT report contains banner");
    ASSERT_TEST(strstr(txt_buf, "ko-fi.com/roguebyte") != NULL, "TXT report contains RogueByte Ko-fi link");
}

/* 7. Test Disconnection Handling */
void test_controller_disconnection(void) {
    printf("\n=== TEST: Controller Disconnection & Fallbacks ===\n");
    DS4Controller ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.connected = false;
    ctrl.handle = -1;
    
    ControllerHidInfo info;
    hid_info_query(&ctrl, &info);
    
    ASSERT_TEST(strcmp(info.connection, "DISCONNECTED") == 0, "Proper disconnection reported");
    ASSERT_TEST(strcmp(info.battery, "NOT AVAILABLE") == 0, "Battery marked NOT AVAILABLE when disconnected");
    ASSERT_TEST(strcmp(info.firmware, "NOT AVAILABLE") == 0, "Firmware marked NOT AVAILABLE");
    ASSERT_TEST(strcmp(info.serial, "NOT AVAILABLE") == 0, "Serial marked NOT AVAILABLE");
}

int main(void) {
    printf("====================================================\n");
    printf("  RogueByte PS4 Controller Lab - Automated Test Suite \n");
    printf("====================================================\n");
    
    test_normalization();
    test_deadzone_math();
    test_drift_math();
    test_range_circularity();
    test_calibration_profile();
    test_report_generation();
    test_controller_disconnection();
    
    printf("\n====================================================\n");
    printf("  TEST RESULTS: %d PASSED, %d FAILED\n", g_tests_passed, g_tests_failed);
    printf("====================================================\n");
    
    return g_tests_failed == 0 ? 0 : 1;
}
