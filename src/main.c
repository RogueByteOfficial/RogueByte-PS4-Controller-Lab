#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "controller/controller.h"
#include "joystick/joystick.h"
#include "calibration/calibration.h"
#include "vibration/vibration.h"
#include "touchpad/touchpad.h"
#include "diagnostics/diagnostics.h"
#include "storage/storage.h"
#include "ui/graphics.h"
#include "ui/ui.h"

#if defined(__ORBIS__) || defined(__PS4__)
#include <kernel.h>
#include <video_out.h>

static int g_video_handle = -1;
static void* g_video_buffers[2] = {NULL, NULL};
static int g_current_buffer_idx = 0;

static int ps4_video_init(void) {
    g_video_handle = sceVideoOutOpen(0, 0, 0, NULL);
    if (g_video_handle < 0) return -1;
    
    /* Allocate 1080p display buffers */
    size_t buf_size = SCREEN_WIDTH * SCREEN_HEIGHT * 4;
    g_video_buffers[0] = malloc(buf_size);
    g_video_buffers[1] = malloc(buf_size);
    if (!g_video_buffers[0] || !g_video_buffers[1]) return -1;
    
    memset(g_video_buffers[0], 0, buf_size);
    memset(g_video_buffers[1], 0, buf_size);
    
    return 0;
}

static void ps4_video_flip(void) {
    if (g_video_handle < 0) return;
    sceVideoOutSubmitFlip(g_video_handle, g_current_buffer_idx, 1 /* flip on vsync */, 0);
    g_current_buffer_idx = 1 - g_current_buffer_idx;
}

static void* ps4_get_current_buffer(void) {
    return g_video_buffers[g_current_buffer_idx];
}

#else
/* Desktop / Standalone compilation mode */
static void* s_framebuffer_mem = NULL;

static int standalone_video_init(void) {
    s_framebuffer_mem = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 4);
    return s_framebuffer_mem ? 0 : -1;
}

static void* standalone_get_buffer(void) {
    return s_framebuffer_mem;
}
#endif

int main(int argc, char* argv[]) {
    printf("====================================================\n");
    printf("Starting RogueByte PS4 Controller Lab (Native App)...\n");
    printf("Developed with ❤️ by Sido dev | RogueByte\n");
    printf("Ko-fi:  https://ko-fi.com/roguebyte\n");
    printf("GitHub: https://github.com/RogueByteOfficial\n");
    printf("====================================================\n");
    
    /* Initialize Storage */
    storage_init();
    AppSettings settings;
    storage_load_settings(&settings);
    
    /* Initialize Video Framebuffer */
    uint32_t* fb_memory = NULL;
#if defined(__ORBIS__) || defined(__PS4__)
    if (ps4_video_init() != 0) {
        printf("Failed to initialize PS4 video output.\n");
        return 1;
    }
    fb_memory = (uint32_t*)ps4_get_current_buffer();
#else
    if (standalone_video_init() != 0) {
        printf("Failed to allocate framebuffer memory.\n");
        return 1;
    }
    fb_memory = (uint32_t*)standalone_get_buffer();
#endif

    Framebuffer fb;
    gfx_init(&fb, fb_memory, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    /* Initialize Subsystems */
    DS4Controller controller;
    controller_init(&controller);
    
    JoystickAnalyzer joystick_analyzer;
    joystick_analyzer_init(&joystick_analyzer);
    
    CalibrationWizard calibration;
    calibration_init(&calibration);
    
    VibrationState vibration;
    vibration_init(&vibration);
    
    TouchpadState touchpad;
    touchpad_init(&touchpad);
    
    DiagnosticSession diagnostics;
    diagnostics_init(&diagnostics);
    
    UIState ui;
    ui_init(&ui);
    ui_show_toast(&ui, "RogueByte PS4 Controller Lab Initialized", 120);
    
    int running = 1;
    int max_frames = (argc > 1 && strcmp(argv[1], "--test-run") == 0) ? 60 : -1;
    int frames_rendered = 0;
    
    while (running) {
        /* Poll DualShock 4 inputs */
        controller_poll(&controller);
        
        /* Update Stick telemetry */
        joystick_analyzer_update(&joystick_analyzer, 
                                 controller.lx, controller.ly, 
                                 controller.rx, controller.ry);
        
        /* Step Drift test if running */
        if (joystick_analyzer.left_drift.state == DRIFT_STATE_SAMPLING) {
            bool done = joystick_drift_test_step(&joystick_analyzer, 
                                                 controller.lx, controller.ly, 
                                                 controller.rx, controller.ry);
            if (done) {
                diagnostics.left_drift_detected = joystick_analyzer.left_drift.drift_detected;
                diagnostics.right_drift_detected = joystick_analyzer.right_drift.drift_detected;
                diagnostics.left_drift_deviation_pct = joystick_analyzer.left_drift.max_deviation_pct;
                diagnostics.right_drift_deviation_pct = joystick_analyzer.right_drift.max_deviation_pct;
                diagnostics.left_drift_result = joystick_analyzer.left_drift.status;
                diagnostics.right_drift_result = joystick_analyzer.right_drift.status;
                ui_show_toast(&ui, "Drift Test Complete!", 120);
            }
        }
        
        /* Step Calibration if active */
        if (ui.current_view == VIEW_CALIBRATION_WIZARD) {
            calibration_step_update(&calibration, 
                                    controller.lx, controller.ly, 
                                    controller.rx, controller.ry);
        }
        
        /* Update Touchpad */
        touchpad_update(&touchpad, controller.touch_active, controller.touch_clicked, 
                        controller.touch_x, controller.touch_y);
        
        /* Update Vibration timers */
        vibration_update(&vibration, controller.handle);
        
        /* Handle navigation & actions */
        ui_handle_input(&ui, &controller, &joystick_analyzer, 
                        &calibration, &vibration, &touchpad, 
                        &diagnostics, &settings);
        
        /* Exit requested from menu */
        if (ui.current_view == VIEW_MAIN_MENU && ui.menu_cursor == 8 && 
            controller_button_down(&controller, DS4_BTN_CROSS)) {
            running = 0;
        }
        
        /* Render frame */
        ui_render(&ui, &fb, &controller, &joystick_analyzer, 
                  &calibration, &vibration, &touchpad, 
                  &diagnostics, &settings);
        
#if defined(__ORBIS__) || defined(__PS4__)
        ps4_video_flip();
        fb.buffer = (uint32_t*)ps4_get_current_buffer();
#else
        /* Limit to ~60 FPS in standalone mode */
        usleep(16666);
#endif
        
        frames_rendered++;
        if (max_frames > 0 && frames_rendered >= max_frames) {
            printf("Completed test run of %d frames successfully.\n", frames_rendered);
            break;
        }
    }
    
    /* Clean shutdown */
    vibration_stop(&vibration, controller.handle);
    controller_close(&controller);
    storage_save_settings(&settings);
    
    printf("RogueByte PS4 Controller Lab shutdown cleanly.\n");
    return 0;
}
