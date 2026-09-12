#include "storage.h"
#include <stdio.h>
#include <sys/stat.h>

void storage_init(void) {
    mkdir("/data", 0777);
    mkdir(BASE_DATA_DIR, 0777);
    mkdir(BASE_DATA_DIR "/reports", 0777);
    mkdir(BASE_DATA_DIR "/profiles", 0777);
}

int storage_save_settings(const AppSettings* settings) {
    if (!settings) return -1;
    storage_init();
    
    FILE* f = fopen(BASE_DATA_DIR "/settings.bin", "wb");
    if (!f) return -1;
    
    fwrite(settings, sizeof(AppSettings), 1, f);
    fclose(f);
    return 0;
}

int storage_load_settings(AppSettings* settings) {
    if (!settings) return -1;
    FILE* f = fopen(BASE_DATA_DIR "/settings.bin", "rb");
    if (!f) {
        settings->deadzone_threshold = 0.08f;
        settings->invert_y_axis = false;
        settings->rumble_enabled = true;
        settings->auto_save_reports = 1;
        return 0;
    }
    
    size_t read_bytes = fread(settings, sizeof(AppSettings), 1, f);
    fclose(f);
    if (read_bytes != 1) {
        settings->deadzone_threshold = 0.08f;
        settings->invert_y_axis = false;
        settings->rumble_enabled = true;
        settings->auto_save_reports = 1;
    }
    return 0;
}
