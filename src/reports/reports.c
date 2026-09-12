#include "reports.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

int report_generate_json(const DS4Controller* ctrl, const DiagnosticSession* diag, char* out_buf, size_t max_len) {
    if (!out_buf || max_len == 0) return -1;
    
    time_t now = time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S UTC", gmtime(&now));
    
    return snprintf(out_buf, max_len,
        "{\n"
        "  \"application\": \"RogueByte PS4 Controller Lab\",\n"
        "  \"version\": \"1.0.0\",\n"
        "  \"developer\": \"RogueByte (Sido dev)\",\n"
        "  \"timestamp\": \"%s\",\n"
        "  \"controller\": {\n"
        "    \"device\": \"%s\",\n"
        "    \"connection\": \"%s\",\n"
        "    \"vendor_id\": \"0x%04X\",\n"
        "    \"product_id\": \"0x%04X\"\n"
        "  },\n"
        "  \"diagnostic_summary\": {\n"
        "    \"buttons\": \"%s\",\n"
        "    \"buttons_count\": \"%d/%d\",\n"
        "    \"left_stick\": \"%s\",\n"
        "    \"right_stick\": \"%s\",\n"
        "    \"drift\": {\n"
        "      \"left_drift_detected\": %s,\n"
        "      \"left_deviation_pct\": %.2f,\n"
        "      \"right_drift_detected\": %s,\n"
        "      \"right_deviation_pct\": %.2f\n"
        "    },\n"
        "    \"touchpad\": \"%s\",\n"
        "    \"vibration\": \"%s\",\n"
        "    \"overall\": \"%s\"\n"
        "  }\n"
        "}\n",
        time_str,
        ctrl ? ctrl->device_name : "Unknown",
        ctrl ? ctrl->connection_type : "Unknown",
        ctrl ? ctrl->vendor_id : 0,
        ctrl ? ctrl->product_id : 0,
        diagnostics_result_to_string(diag->buttons_result),
        diag->buttons_passed_count, diag->buttons_total_count,
        diagnostics_result_to_string(diag->left_stick_result),
        diagnostics_result_to_string(diag->right_stick_result),
        diag->left_drift_detected ? "true" : "false",
        diag->left_drift_deviation_pct,
        diag->right_drift_detected ? "true" : "false",
        diag->right_drift_deviation_pct,
        diagnostics_result_to_string(diag->touchpad_result),
        diagnostics_result_to_string(diag->vibration_result),
        diagnostics_result_to_string(diag->overall_result)
    );
}

int report_generate_text(const DS4Controller* ctrl, const DiagnosticSession* diag, char* out_buf, size_t max_len) {
    if (!out_buf || max_len == 0) return -1;
    
    time_t now = time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S UTC", gmtime(&now));
    
    return snprintf(out_buf, max_len,
        "==========================================================\n"
        "             ROGUEBYTE PS4 CONTROLLER LAB                 \n"
        "                 DIAGNOSTIC REPORT                        \n"
        "==========================================================\n"
        "Timestamp:   %s\n"
        "Controller:  %s\n"
        "Connection:  %s\n"
        "VID / PID:   0x%04X / 0x%04X\n"
        "----------------------------------------------------------\n"
        "TEST RESULTS:\n"
        "  BUTTONS:     [%-7s] (%d/%d inputs registered)\n"
        "  LEFT STICK:  [%-7s] (Range: %.1f%%)\n"
        "  RIGHT STICK: [%-7s] (Range: %.1f%%)\n"
        "  DRIFT TEST:\n"
        "    - LEFT:    [%-7s] Dev: %.2f%%\n"
        "    - RIGHT:   [%-7s] Dev: %.2f%%\n"
        "  TOUCHPAD:    [%-7s]\n"
        "  VIBRATION:   [%-7s]\n"
        "----------------------------------------------------------\n"
        "OVERALL HEALTH EVALUATION: [%s]\n"
        "==========================================================\n"
        "Developer: RogueByte (Developed with ❤️ by Sido dev)\n"
        "Ko-fi:     https://ko-fi.com/roguebyte\n"
        "GitHub:    https://github.com/RogueByteOfficial\n"
        "Dedicated to the PlayStation gaming & repair community.\n"
        "==========================================================\n",
        time_str,
        ctrl ? ctrl->device_name : "Unknown",
        ctrl ? ctrl->connection_type : "Unknown",
        ctrl ? ctrl->vendor_id : 0,
        ctrl ? ctrl->product_id : 0,
        diagnostics_result_to_string(diag->buttons_result),
        diag->buttons_passed_count, diag->buttons_total_count,
        diagnostics_result_to_string(diag->left_stick_result), diag->left_range_pct,
        diagnostics_result_to_string(diag->right_stick_result), diag->right_range_pct,
        diag->left_drift_detected ? "FAIL" : "PASS", diag->left_drift_deviation_pct,
        diag->right_drift_detected ? "FAIL" : "PASS", diag->right_drift_deviation_pct,
        diagnostics_result_to_string(diag->touchpad_result),
        diagnostics_result_to_string(diag->vibration_result),
        diagnostics_result_to_string(diag->overall_result)
    );
}

int report_save_to_disk(const char* dir_path, const char* json_data, const char* txt_data) {
    if (!dir_path) dir_path = "/data/RogueByte_Controller_Lab/reports";
    
    mkdir("/data", 0777);
    mkdir("/data/RogueByte_Controller_Lab", 0777);
    mkdir(dir_path, 0777);
    
    time_t now = time(NULL);
    char json_filename[256];
    char txt_filename[256];
    
    snprintf(json_filename, sizeof(json_filename), "%s/report_%ld.json", dir_path, (long)now);
    snprintf(txt_filename, sizeof(txt_filename), "%s/report_%ld.txt", dir_path, (long)now);
    
    if (json_data) {
        FILE* fj = fopen(json_filename, "w");
        if (fj) {
            fputs(json_data, fj);
            fclose(fj);
        }
    }
    
    if (txt_data) {
        FILE* ft = fopen(txt_filename, "w");
        if (ft) {
            fputs(txt_data, ft);
            fclose(ft);
        }
    }
    
    return 0;
}
