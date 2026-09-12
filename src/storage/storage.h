#ifndef ROGUEBYTE_STORAGE_H
#define ROGUEBYTE_STORAGE_H

#include <stdbool.h>

#define BASE_DATA_DIR "/data/RogueByte_Controller_Lab"

typedef struct {
    float deadzone_threshold;
    bool invert_y_axis;
    bool rumble_enabled;
    int auto_save_reports;
} AppSettings;

void storage_init(void);
int storage_save_settings(const AppSettings* settings);
int storage_load_settings(AppSettings* settings);

#endif /* ROGUEBYTE_STORAGE_H */
