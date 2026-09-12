#include "ui.h"
#include "../hid/hid_info.h"
#include "../reports/reports.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static const char* s_menu_items[] = {
    "Controller Test",
    "Joystick Analyzer",
    "Joystick Calibration",
    "Vibration Test",
    "Touchpad Test",
    "Controller Information",
    "Diagnostic Report",
    "Settings",
    "Exit"
};
#define MENU_ITEM_COUNT 9

void ui_init(UIState* ui) {
    if (!ui) return;
    memset(ui, 0, sizeof(UIState));
    ui->current_view = VIEW_MAIN_MENU;
    ui->previous_view = VIEW_MAIN_MENU;
    ui->menu_cursor = 0;
    ui->deadzone_cursor = 0;
    ui->vibration_cursor = 0;
    ui->settings_cursor = 0;
}

void ui_show_toast(UIState* ui, const char* msg, int duration_frames) {
    if (!ui || !msg) return;
    strncpy(ui->toast_message, msg, sizeof(ui->toast_message) - 1);
    ui->toast_timer = duration_frames > 0 ? duration_frames : 180;
}

/* Helper draw functions */
static void draw_header(Framebuffer* fb, const char* title, const DS4Controller* ctrl) {
    gfx_fill_rect(fb, 0, 0, SCREEN_WIDTH, 80, COLOR_PANEL);
    gfx_fill_rect(fb, 0, 78, SCREEN_WIDTH, 2, COLOR_CYAN);
    
    /* Brand logo & title */
    gfx_draw_string(fb, 60, 26, "ROGUEBYTE", COLOR_CYAN, 2);
    gfx_draw_string(fb, 260, 26, "|", COLOR_BORDER, 2);
    gfx_draw_string(fb, 285, 26, title, COLOR_TEXT_WHITE, 2);
    
    /* Connection Badge */
    int badge_x = SCREEN_WIDTH - 380;
    if (ctrl && ctrl->connected) {
        gfx_fill_rounded_rect(fb, badge_x, 20, 320, 40, 8, COLOR_RGB(6, 78, 59));
        gfx_draw_rounded_rect(fb, badge_x, 20, 320, 40, 8, COLOR_PASS);
        gfx_fill_circle(fb, badge_x + 20, 40, 6, COLOR_PASS);
        gfx_draw_string(fb, badge_x + 35, 30, "DUALSHOCK 4 : CONNECTED", COLOR_TEXT_WHITE, 1);
    } else {
        gfx_fill_rounded_rect(fb, badge_x, 20, 320, 40, 8, COLOR_RGB(127, 29, 29));
        gfx_draw_rounded_rect(fb, badge_x, 20, 320, 40, 8, COLOR_FAIL);
        gfx_fill_circle(fb, badge_x + 20, 40, 6, COLOR_FAIL);
        gfx_draw_string(fb, badge_x + 35, 30, "NO CONTROLLER DETECTED", COLOR_TEXT_WHITE, 1);
    }
}

static void draw_footer(Framebuffer* fb, const UIState* ui) {
    gfx_fill_rect(fb, 0, SCREEN_HEIGHT - 60, SCREEN_WIDTH, 60, COLOR_PANEL);
    gfx_fill_rect(fb, 0, SCREEN_HEIGHT - 60, SCREEN_WIDTH, 1, COLOR_BORDER);
    
    /* Navigation Hint */
    const char* hint = "[D-Pad/Stick] Navigate   [X] Select   [O] Back   [Options] Quick Settings";
    if (ui->current_view == VIEW_CONTROLLER_TEST) {
        hint = "Press all 18 buttons to complete test | [O] Back to Menu";
    } else if (ui->current_view == VIEW_DRIFT_DETECTOR) {
        hint = "[X] Start Drift Test | Release Sticks | [O] Back";
    } else if (ui->current_view == VIEW_CALIBRATION_WIZARD) {
        hint = "[X] Next Step | [O] Cancel Calibration";
    }
    gfx_draw_string(fb, 60, SCREEN_HEIGHT - 38, hint, COLOR_TEXT_MUTED, 1);
    
    /* Developer signature */
    gfx_draw_string(fb, SCREEN_WIDTH - 680, SCREEN_HEIGHT - 38, 
                    "Developed with ❤️ by Sido dev | RogueByte (ko-fi.com/roguebyte)", 
                    COLOR_CYAN, 1);
}

static void draw_toast(Framebuffer* fb, UIState* ui) {
    if (ui->toast_timer > 0) {
        ui->toast_timer--;
        int tw = 600;
        int th = 50;
        int tx = (SCREEN_WIDTH - tw) / 2;
        int ty = 100;
        gfx_fill_rounded_rect(fb, tx, ty, tw, th, 8, COLOR_PANEL_ALT);
        gfx_draw_rounded_rect(fb, tx, ty, tw, th, 8, COLOR_CYAN);
        gfx_draw_string_centered(fb, SCREEN_WIDTH / 2, ty + 16, ui->toast_message, COLOR_CYAN, 1);
    }
}

/* Render Main Menu */
static void render_main_menu(Framebuffer* fb, const UIState* ui, const DS4Controller* ctrl) {
    /* Menu container */
    int start_x = 120;
    int start_y = 160;
    int item_w = 580;
    int item_h = 72;
    int spacing = 18;
    
    gfx_draw_string(fb, start_x, start_y - 35, "SELECT DIAGNOSTIC MODULE", COLOR_TEXT_MUTED, 1);
    
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        int y = start_y + i * (item_h + spacing);
        bool is_selected = (ui->menu_cursor == i);
        
        if (is_selected) {
            gfx_fill_rounded_rect(fb, start_x, y, item_w, item_h, 8, COLOR_BTN_ACTIVE);
            gfx_draw_rounded_rect(fb, start_x, y, item_w, item_h, 8, COLOR_CYAN);
            gfx_fill_rect(fb, start_x, y + 10, 8, item_h - 20, COLOR_CYAN);
            gfx_draw_string(fb, start_x + 35, y + 24, s_menu_items[i], COLOR_TEXT_WHITE, 2);
            gfx_draw_string(fb, start_x + item_w - 60, y + 26, ">", COLOR_TEXT_WHITE, 2);
        } else {
            gfx_fill_rounded_rect(fb, start_x, y, item_w, item_h, 8, COLOR_PANEL);
            gfx_draw_rounded_rect(fb, start_x, y, item_w, item_h, 8, COLOR_BORDER);
            gfx_draw_string(fb, start_x + 35, y + 24, s_menu_items[i], COLOR_TEXT_MUTED, 2);
        }
    }
    
    /* Right Overview Dashboard Panel */
    int dash_x = 760;
    int dash_y = 160;
    int dash_w = 1040;
    int dash_h = 790;
    gfx_fill_rounded_rect(fb, dash_x, dash_y, dash_w, dash_h, 12, COLOR_PANEL);
    gfx_draw_rounded_rect(fb, dash_x, dash_y, dash_w, dash_h, 12, COLOR_BORDER);
    
    gfx_draw_string(fb, dash_x + 40, dash_y + 40, "CONTROLLER HARDWARE STATUS", COLOR_CYAN, 2);
    gfx_fill_rect(fb, dash_x + 40, dash_y + 75, dash_w - 80, 1, COLOR_BORDER);
    
    /* Specs Cards */
    char buf[128];
    int info_y = dash_y + 110;
    
    snprintf(buf, sizeof(buf), "Device Model:      %s", ctrl ? ctrl->device_name : "Not Detected");
    gfx_draw_string(fb, dash_x + 40, info_y, buf, COLOR_TEXT_WHITE, 1);
    
    snprintf(buf, sizeof(buf), "Vendor ID:         0x%04X (Sony Interactive Entertainment)", ctrl ? ctrl->vendor_id : 0);
    gfx_draw_string(fb, dash_x + 40, info_y + 40, buf, COLOR_TEXT_WHITE, 1);
    
    snprintf(buf, sizeof(buf), "Product ID:        0x%04X (%s)", 
             ctrl ? ctrl->product_id : 0, 
             ctrl && ctrl->product_id == 0x09CC ? "Gen 2 Slim/Pro" : "Gen 1 Standard");
    gfx_draw_string(fb, dash_x + 40, info_y + 80, buf, COLOR_TEXT_WHITE, 1);
    
    snprintf(buf, sizeof(buf), "Connection Mode:   %s (Polling rate: 250Hz)", ctrl ? ctrl->connection_type : "Unknown");
    gfx_draw_string(fb, dash_x + 40, info_y + 120, buf, COLOR_PASS, 1);
    
    snprintf(buf, sizeof(buf), "Firmware Version:  NOT AVAILABLE (Protected by PS4 user sandbox)");
    gfx_draw_string(fb, dash_x + 40, info_y + 160, buf, COLOR_WARN, 1);
    
    snprintf(buf, sizeof(buf), "Hardware Serial:   NOT AVAILABLE (Factory locked EEPROM)");
    gfx_draw_string(fb, dash_x + 40, info_y + 200, buf, COLOR_WARN, 1);
    
    /* Live Analog Preview inside overview */
    gfx_draw_string(fb, dash_x + 40, dash_y + 380, "LIVE THUMBSTICKS SNAPSHOT", COLOR_CYAN, 2);
    gfx_fill_rect(fb, dash_x + 40, dash_y + 415, dash_w - 80, 1, COLOR_BORDER);
    
    int ls_cx = dash_x + 240;
    int stick_cy = dash_y + 560;
    int rs_cx = dash_x + 680;
    int rad = 90;
    
    /* Left stick */
    gfx_draw_circle(fb, ls_cx, stick_cy, rad, COLOR_BORDER);
    gfx_draw_circle(fb, ls_cx, stick_cy, 12, COLOR_RGB(40, 60, 90));
    gfx_draw_line(fb, ls_cx - rad, stick_cy, ls_cx + rad, stick_cy, COLOR_RGB(30, 45, 70));
    gfx_draw_line(fb, ls_cx, stick_cy - rad, ls_cx, stick_cy + rad, COLOR_RGB(30, 45, 70));
    
    int ls_dot_x = ls_cx + (int)(((float)(ctrl ? ctrl->lx : 128) - 128.0f) * (rad / 128.0f));
    int ls_dot_y = stick_cy + (int)(((float)(ctrl ? ctrl->ly : 128) - 128.0f) * (rad / 128.0f));
    gfx_fill_circle(fb, ls_dot_x, ls_dot_y, 10, COLOR_CYAN);
    
    gfx_draw_string_centered(fb, ls_cx, stick_cy + rad + 20, "LEFT STICK", COLOR_TEXT_WHITE, 1);
    snprintf(buf, sizeof(buf), "X: %3d  Y: %3d", ctrl ? ctrl->lx : 128, ctrl ? ctrl->ly : 128);
    gfx_draw_string_centered(fb, ls_cx, stick_cy + rad + 45, buf, COLOR_TEXT_MUTED, 1);
    
    /* Right stick */
    gfx_draw_circle(fb, rs_cx, stick_cy, rad, COLOR_BORDER);
    gfx_draw_circle(fb, rs_cx, stick_cy, 12, COLOR_RGB(40, 60, 90));
    gfx_draw_line(fb, rs_cx - rad, stick_cy, rs_cx + rad, stick_cy, COLOR_RGB(30, 45, 70));
    gfx_draw_line(fb, rs_cx, stick_cy - rad, rs_cx, stick_cy + rad, COLOR_RGB(30, 45, 70));
    
    int rs_dot_x = rs_cx + (int)(((float)(ctrl ? ctrl->rx : 128) - 128.0f) * (rad / 128.0f));
    int rs_dot_y = stick_cy + (int)(((float)(ctrl ? ctrl->ry : 128) - 128.0f) * (rad / 128.0f));
    gfx_fill_circle(fb, rs_dot_x, rs_dot_y, 10, COLOR_PASS);
    
    gfx_draw_string_centered(fb, rs_cx, stick_cy + rad + 20, "RIGHT STICK", COLOR_TEXT_WHITE, 1);
    snprintf(buf, sizeof(buf), "X: %3d  Y: %3d", ctrl ? ctrl->rx : 128, ctrl ? ctrl->ry : 128);
    gfx_draw_string_centered(fb, rs_cx, stick_cy + rad + 45, buf, COLOR_TEXT_MUTED, 1);
}

/* Render Controller Test */
static void render_controller_test(Framebuffer* fb, const DS4Controller* ctrl) {
    int tested = controller_get_tested_button_count(ctrl);
    char buf[64];
    
    /* Score Banner */
    gfx_fill_rounded_rect(fb, 120, 110, 1680, 70, 8, COLOR_PANEL);
    gfx_draw_rounded_rect(fb, 120, 110, 1680, 70, 8, COLOR_BORDER);
    snprintf(buf, sizeof(buf), "TEST PROGRESS: %d / %d INPUTS VERIFIED", tested, TOTAL_TESTABLE_BUTTONS);
    gfx_draw_string(fb, 160, 132, buf, tested == TOTAL_TESTABLE_BUTTONS ? COLOR_PASS : COLOR_CYAN, 2);
    
    if (tested == TOTAL_TESTABLE_BUTTONS) {
        gfx_draw_string(fb, 1400, 132, "[ STATUS: ALL PASS ]", COLOR_PASS, 2);
    }
    
    /* Button Matrix Grid Cards */
    struct {
        const char* label;
        uint32_t mask;
        int x, y, w, h;
    } buttons[] = {
        {"[L1]",       DS4_BTN_L1,       280,  240, 160, 60},
        {"[L2]",       DS4_BTN_L2,       280,  320, 160, 60},
        {"[R1]",       DS4_BTN_R1,      1480,  240, 160, 60},
        {"[R2]",       DS4_BTN_R2,      1480,  320, 160, 60},
        
        {"D-PAD UP",   DS4_BTN_UP,       420,  440, 140, 60},
        {"D-PAD DOWN", DS4_BTN_DOWN,     420,  580, 140, 60},
        {"D-PAD LEFT", DS4_BTN_LEFT,     260,  510, 140, 60},
        {"D-PAD RIGHT",DS4_BTN_RIGHT,    580,  510, 140, 60},
        
        {"TRIANGLE (△)",DS4_BTN_TRIANGLE, 1360, 440, 160, 60},
        {"CROSS (X)",  DS4_BTN_CROSS,    1360, 580, 160, 60},
        {"SQUARE (□)", DS4_BTN_SQUARE,   1180, 510, 160, 60},
        {"CIRCLE (O)", DS4_BTN_CIRCLE,   1540, 510, 160, 60},
        
        {"SHARE",      DS4_BTN_SHARE,    720,  360, 130, 50},
        {"OPTIONS",    DS4_BTN_OPTIONS,  1070, 360, 130, 50},
        {"TOUCHPAD",   DS4_BTN_TOUCHPAD, 850,  280, 220, 90},
        {"[PS]",       DS4_BTN_PS,       920,  620,  80, 50},
        
        {"L3 (STICK)", DS4_BTN_L3,       580,  700, 180, 65},
        {"R3 (STICK)", DS4_BTN_R3,      1160,  700, 180, 65},
    };
    
    for (size_t i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++) {
        bool is_pressed = ctrl && (ctrl->buttons & buttons[i].mask);
        bool has_tested = ctrl && (ctrl->buttons_tested & buttons[i].mask);
        
        uint32_t bg = is_pressed ? COLOR_PASS : (has_tested ? COLOR_PANEL_ALT : COLOR_PANEL);
        uint32_t border = is_pressed ? COLOR_PASS : (has_tested ? COLOR_CYAN : COLOR_BORDER);
        uint32_t text_col = is_pressed ? COLOR_RGB(10, 30, 20) : (has_tested ? COLOR_TEXT_WHITE : COLOR_TEXT_MUTED);
        
        gfx_fill_rounded_rect(fb, buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, 8, bg);
        gfx_draw_rounded_rect(fb, buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, 8, border);
        gfx_draw_string_centered(fb, buttons[i].x + buttons[i].w / 2, buttons[i].y + buttons[i].h / 2 - 8, 
                                 buttons[i].label, text_col, 1);
    }
    
    /* L2 / R2 Analog Pressure Bars */
    gfx_draw_string(fb, 280, 400, "L2 Pressure:", COLOR_TEXT_MUTED, 1);
    gfx_fill_rect(fb, 410, 400, 200, 16, COLOR_PANEL);
    gfx_draw_rect(fb, 410, 400, 200, 16, COLOR_BORDER);
    int l2_w = (int)(((float)(ctrl ? ctrl->l2_analog : 0) / 255.0f) * 200);
    gfx_fill_rect(fb, 410, 400, l2_w, 16, COLOR_CYAN);
    
    gfx_draw_string(fb, 1280, 400, "R2 Pressure:", COLOR_TEXT_MUTED, 1);
    gfx_fill_rect(fb, 1410, 400, 200, 16, COLOR_PANEL);
    gfx_draw_rect(fb, 1410, 400, 200, 16, COLOR_BORDER);
    int r2_w = (int)(((float)(ctrl ? ctrl->r2_analog : 0) / 255.0f) * 200);
    gfx_fill_rect(fb, 1410, 400, r2_w, 16, COLOR_CYAN);
    
    /* Clear Test Button reminder */
    gfx_draw_string_centered(fb, SCREEN_WIDTH / 2, 850, 
                             "[X] Reset Tested Matrix   |   [O] Return to Main Menu", 
                             COLOR_TEXT_MUTED, 1);
}

/* Render Joystick Analyzer */
static void render_joystick_analyzer(Framebuffer* fb, const DS4Controller* ctrl, const JoystickAnalyzer* ja) {
    /* Left and Right stick analyzers side-by-side */
    int panel_w = 800;
    int panel_h = 780;
    int ly_panel_x = 120;
    int ry_panel_x = 1000;
    int panel_y = 120;
    
    char buf[128];
    
    /* Left Stick Panel */
    gfx_fill_rounded_rect(fb, ly_panel_x, panel_y, panel_w, panel_h, 12, COLOR_PANEL);
    gfx_draw_rounded_rect(fb, ly_panel_x, panel_y, panel_w, panel_h, 12, COLOR_BORDER);
    gfx_draw_string(fb, ly_panel_x + 40, panel_y + 30, "LEFT ANALOG STICK", COLOR_CYAN, 2);
    
    /* Left Reticle */
    int lcx = ly_panel_x + 220;
    int lcy = panel_y + 260;
    int r = 150;
    
    gfx_draw_circle(fb, lcx, lcy, r, COLOR_BORDER);
    gfx_draw_circle(fb, lcx, lcy, 8, COLOR_RGB(30, 45, 70));
    /* Deadzone circle */
    int l_deadzone_r = (int)(r * (ja ? ja->user_deadzone_threshold : 0.08f));
    gfx_draw_circle(fb, lcx, lcy, l_deadzone_r, COLOR_WARN);
    
    gfx_draw_line(fb, lcx - r, lcy, lcx + r, lcy, COLOR_RGB(25, 38, 58));
    gfx_draw_line(fb, lcx, lcy - r, lcx, lcy + r, COLOR_RGB(25, 38, 58));
    
    int l_x = lcx + (int)(((float)(ctrl ? ctrl->lx : 128) - 128.0f) * (r / 128.0f));
    int l_y = lcy + (int)(((float)(ctrl ? ctrl->ly : 128) - 128.0f) * (r / 128.0f));
    gfx_fill_circle(fb, l_x, l_y, 12, COLOR_CYAN);
    
    /* Left Data Box */
    int l_data_x = ly_panel_x + 420;
    int l_data_y = panel_y + 110;
    
    snprintf(buf, sizeof(buf), "Raw X:      %d", ctrl ? ctrl->lx : 128);
    gfx_draw_string(fb, l_data_x, l_data_y, buf, COLOR_TEXT_WHITE, 1);
    snprintf(buf, sizeof(buf), "Raw Y:      %d", ctrl ? ctrl->ly : 128);
    gfx_draw_string(fb, l_data_x, l_data_y + 30, buf, COLOR_TEXT_WHITE, 1);
    
    snprintf(buf, sizeof(buf), "Norm X:     %+.3f", ja ? ja->left_current.norm_x : 0.0f);
    gfx_draw_string(fb, l_data_x, l_data_y + 70, buf, COLOR_CYAN, 1);
    snprintf(buf, sizeof(buf), "Norm Y:     %+.3f", ja ? ja->left_current.norm_y : 0.0f);
    gfx_draw_string(fb, l_data_x, l_data_y + 100, buf, COLOR_CYAN, 1);
    
    snprintf(buf, sizeof(buf), "Magnitude:  %.3f", ja ? ja->left_current.magnitude : 0.0f);
    gfx_draw_string(fb, l_data_x, l_data_y + 140, buf, COLOR_TEXT_WHITE, 1);
    snprintf(buf, sizeof(buf), "Deadzone:   %.1f%% (Area)", (ja ? ja->user_deadzone_threshold : 0.08f) * 100.0f);
    gfx_draw_string(fb, l_data_x, l_data_y + 170, buf, COLOR_WARN, 1);
    
    snprintf(buf, sizeof(buf), "Min X / Max X:  %d / %d", ja ? ja->left_stats.min_raw_x : 0, ja ? ja->left_stats.max_raw_x : 255);
    gfx_draw_string(fb, l_data_x, l_data_y + 210, buf, COLOR_TEXT_MUTED, 1);
    snprintf(buf, sizeof(buf), "Min Y / Max Y:  %d / %d", ja ? ja->left_stats.min_raw_y : 0, ja ? ja->left_stats.max_raw_y : 255);
    gfx_draw_string(fb, l_data_x, l_data_y + 240, buf, COLOR_TEXT_MUTED, 1);
    
    float l_dev = fabsf(ja ? ja->left_current.magnitude : 0.0f);
    bool l_drifting = (l_dev > (ja ? ja->user_deadzone_threshold : 0.08f)) && 
                      (ctrl && ctrl->lx != 128 && ctrl->ly != 128);
    gfx_draw_string(fb, ly_panel_x + 40, panel_y + 540, "DRIFT STATUS:", COLOR_TEXT_WHITE, 2);
    if (l_drifting) {
        gfx_draw_string(fb, ly_panel_x + 240, panel_y + 540, "DRIFT DETECTED", COLOR_FAIL, 2);
    } else {
        gfx_draw_string(fb, ly_panel_x + 240, panel_y + 540, "NONE (STABLE)", COLOR_PASS, 2);
    }
    
    /* Right Stick Panel */
    gfx_fill_rounded_rect(fb, ry_panel_x, panel_y, panel_w, panel_h, 12, COLOR_PANEL);
    gfx_draw_rounded_rect(fb, ry_panel_x, panel_y, panel_w, panel_h, 12, COLOR_BORDER);
    gfx_draw_string(fb, ry_panel_x + 40, panel_y + 30, "RIGHT ANALOG STICK", COLOR_PASS, 2);
    
    /* Right Reticle */
    int rcx = ry_panel_x + 220;
    int rcy = panel_y + 260;
    
    gfx_draw_circle(fb, rcx, rcy, r, COLOR_BORDER);
    gfx_draw_circle(fb, rcx, rcy, 8, COLOR_RGB(30, 45, 70));
    gfx_draw_circle(fb, rcx, rcy, l_deadzone_r, COLOR_WARN);
    
    gfx_draw_line(fb, rcx - r, rcy, rcx + r, rcy, COLOR_RGB(25, 38, 58));
    gfx_draw_line(fb, rcx, rcy - r, rcx, rcy + r, COLOR_RGB(25, 38, 58));
    
    int r_x = rcx + (int)(((float)(ctrl ? ctrl->rx : 128) - 128.0f) * (r / 128.0f));
    int r_y = rcy + (int)(((float)(ctrl ? ctrl->ry : 128) - 128.0f) * (r / 128.0f));
    gfx_fill_circle(fb, r_x, r_y, 12, COLOR_PASS);
    
    /* Right Data Box */
    int r_data_x = ry_panel_x + 420;
    int r_data_y = panel_y + 110;
    
    snprintf(buf, sizeof(buf), "Raw X:      %d", ctrl ? ctrl->rx : 128);
    gfx_draw_string(fb, r_data_x, r_data_y, buf, COLOR_TEXT_WHITE, 1);
    snprintf(buf, sizeof(buf), "Raw Y:      %d", ctrl ? ctrl->ry : 128);
    gfx_draw_string(fb, r_data_x, r_data_y + 30, buf, COLOR_TEXT_WHITE, 1);
    
    snprintf(buf, sizeof(buf), "Norm X:     %+.3f", ja ? ja->right_current.norm_x : 0.0f);
    gfx_draw_string(fb, r_data_x, r_data_y + 70, buf, COLOR_PASS, 1);
    snprintf(buf, sizeof(buf), "Norm Y:     %+.3f", ja ? ja->right_current.norm_y : 0.0f);
    gfx_draw_string(fb, r_data_x, r_data_y + 100, buf, COLOR_PASS, 1);
    
    snprintf(buf, sizeof(buf), "Magnitude:  %.3f", ja ? ja->right_current.magnitude : 0.0f);
    gfx_draw_string(fb, r_data_x, r_data_y + 140, buf, COLOR_TEXT_WHITE, 1);
    snprintf(buf, sizeof(buf), "Deadzone:   %.1f%% (Area)", (ja ? ja->user_deadzone_threshold : 0.08f) * 100.0f);
    gfx_draw_string(fb, r_data_x, r_data_y + 170, buf, COLOR_WARN, 1);
    
    snprintf(buf, sizeof(buf), "Min X / Max X:  %d / %d", ja ? ja->right_stats.min_raw_x : 0, ja ? ja->right_stats.max_raw_x : 255);
    gfx_draw_string(fb, r_data_x, r_data_y + 210, buf, COLOR_TEXT_MUTED, 1);
    snprintf(buf, sizeof(buf), "Min Y / Max Y:  %d / %d", ja ? ja->right_stats.min_raw_y : 0, ja ? ja->right_stats.max_raw_y : 255);
    gfx_draw_string(fb, r_data_x, r_data_y + 240, buf, COLOR_TEXT_MUTED, 1);
    
    gfx_draw_string(fb, ry_panel_x + 40, panel_y + 540, "DRIFT STATUS:", COLOR_TEXT_WHITE, 2);
    gfx_draw_string(fb, ry_panel_x + 240, panel_y + 540, "NONE (STABLE)", COLOR_PASS, 2);
    
    /* Navigation shortcut */
    gfx_draw_string_centered(fb, SCREEN_WIDTH / 2, 930, 
                             "[Square] Run Deep Drift Test   |   [Triangle] Range Sweep Test   |   [O] Back", 
                             COLOR_CYAN, 1);
}

/* Render Drift Test */
static void render_drift_detector(Framebuffer* fb, const JoystickAnalyzer* ja) {
    int cx = SCREEN_WIDTH / 2;
    int y = 140;
    
    gfx_draw_string_centered(fb, cx, y, "HARDWARE STICK DRIFT DETECTOR", COLOR_CYAN, 3);
    gfx_draw_string_centered(fb, cx, y + 60, "Algorithmic Resting Center & RMS Deviation Sampling", COLOR_TEXT_MUTED, 1);
    
    int card_w = 1200;
    int card_h = 640;
    int card_x = (SCREEN_WIDTH - card_w) / 2;
    int card_y = 240;
    
    gfx_fill_rounded_rect(fb, card_x, card_y, card_w, card_h, 12, COLOR_PANEL);
    gfx_draw_rounded_rect(fb, card_x, card_y, card_w, card_h, 12, COLOR_BORDER);
    
    if (ja && (ja->left_drift.state == DRIFT_STATE_SAMPLING)) {
        gfx_draw_string_centered(fb, cx, card_y + 80, "DO NOT TOUCH THE CONTROLLER", COLOR_FAIL, 3);
        gfx_draw_string_centered(fb, cx, card_y + 150, "Release both analog sticks immediately.", COLOR_TEXT_WHITE, 2);
        gfx_draw_string_centered(fb, cx, card_y + 200, "Acquiring resting position samples at 60Hz...", COLOR_CYAN, 1);
        
        /* Progress bar */
        int pb_w = 800;
        int pb_h = 30;
        int pb_x = (SCREEN_WIDTH - pb_w) / 2;
        int pb_y = card_y + 280;
        
        gfx_fill_rect(fb, pb_x, pb_y, pb_w, pb_h, COLOR_PANEL_ALT);
        gfx_draw_rect(fb, pb_x, pb_y, pb_w, pb_h, COLOR_BORDER);
        
        int fill_w = (int)(((float)ja->left_drift.sample_count / (float)ja->left_drift.max_samples) * pb_w);
        gfx_fill_rect(fb, pb_x, pb_y, fill_w, pb_h, COLOR_CYAN);
        
        char pbuf[32];
        snprintf(pbuf, sizeof(pbuf), "%d / %d Samples", ja->left_drift.sample_count, ja->left_drift.max_samples);
        gfx_draw_string_centered(fb, cx, pb_y + 50, pbuf, COLOR_TEXT_MUTED, 1);
    } else if (ja && ja->left_drift.state == DRIFT_STATE_FINISHED) {
        gfx_draw_string_centered(fb, cx, card_y + 40, "DRIFT TEST COMPLETE", COLOR_PASS, 3);
        
        char res_buf[128];
        /* Left stick results */
        int col1_x = card_x + 100;
        int col_y = card_y + 120;
        gfx_draw_string(fb, col1_x, col_y, "LEFT STICK RESULTS", COLOR_CYAN, 2);
        snprintf(res_buf, sizeof(res_buf), "Center Average: X = %.1f, Y = %.1f", 
                 ja->left_drift.center_avg_x, ja->left_drift.center_avg_y);
        gfx_draw_string(fb, col1_x, col_y + 50, res_buf, COLOR_TEXT_WHITE, 1);
        
        snprintf(res_buf, sizeof(res_buf), "Max Deviation:  %.2f%%", ja->left_drift.max_deviation_pct);
        gfx_draw_string(fb, col1_x, col_y + 90, res_buf, 
                        ja->left_drift.drift_detected ? COLOR_FAIL : COLOR_TEXT_WHITE, 1);
        
        snprintf(res_buf, sizeof(res_buf), "RMS Deviation:  %.2f%%", ja->left_drift.rms_deviation_pct);
        gfx_draw_string(fb, col1_x, col_y + 130, res_buf, COLOR_TEXT_MUTED, 1);
        
        snprintf(res_buf, sizeof(res_buf), "Deadzone Limit: %.1f%%", ja->left_drift.deadzone_threshold * 100.0f);
        gfx_draw_string(fb, col1_x, col_y + 170, res_buf, COLOR_WARN, 1);
        
        gfx_draw_string(fb, col1_x, col_y + 220, "DRIFT VERDICT:", COLOR_TEXT_WHITE, 2);
        if (ja->left_drift.drift_detected) {
            gfx_draw_string(fb, col1_x + 240, col_y + 220, "DRIFT DETECTED", COLOR_FAIL, 2);
        } else {
            gfx_draw_string(fb, col1_x + 240, col_y + 220, "PASS (EXCELLENT)", COLOR_PASS, 2);
        }
        
        /* Right stick results */
        int col2_x = card_x + 650;
        gfx_draw_string(fb, col2_x, col_y, "RIGHT STICK RESULTS", COLOR_PASS, 2);
        snprintf(res_buf, sizeof(res_buf), "Center Average: X = %.1f, Y = %.1f", 
                 ja->right_drift.center_avg_x, ja->right_drift.center_avg_y);
        gfx_draw_string(fb, col2_x, col_y + 50, res_buf, COLOR_TEXT_WHITE, 1);
        
        snprintf(res_buf, sizeof(res_buf), "Max Deviation:  %.2f%%", ja->right_drift.max_deviation_pct);
        gfx_draw_string(fb, col2_x, col_y + 90, res_buf, 
                        ja->right_drift.drift_detected ? COLOR_FAIL : COLOR_TEXT_WHITE, 1);
        
        snprintf(res_buf, sizeof(res_buf), "RMS Deviation:  %.2f%%", ja->right_drift.rms_deviation_pct);
        gfx_draw_string(fb, col2_x, col_y + 130, res_buf, COLOR_TEXT_MUTED, 1);
        
        snprintf(res_buf, sizeof(res_buf), "Deadzone Limit: %.1f%%", ja->right_drift.deadzone_threshold * 100.0f);
        gfx_draw_string(fb, col2_x, col_y + 170, res_buf, COLOR_WARN, 1);
        
        gfx_draw_string(fb, col2_x, col_y + 220, "DRIFT VERDICT:", COLOR_TEXT_WHITE, 2);
        if (ja->right_drift.drift_detected) {
            gfx_draw_string(fb, col2_x + 240, col_y + 220, "DRIFT DETECTED", COLOR_FAIL, 2);
        } else {
            gfx_draw_string(fb, col2_x + 240, col_y + 220, "PASS (EXCELLENT)", COLOR_PASS, 2);
        }
        
        gfx_draw_string_centered(fb, cx, card_y + 480, 
                                 "[X] Re-run Drift Test   |   [O] Back to Main Menu", 
                                 COLOR_CYAN, 1);
    } else {
        /* Idle Prompt */
        gfx_draw_string_centered(fb, cx, card_y + 140, "PREPARE FOR DRIFT MEASUREMENT", COLOR_TEXT_WHITE, 2);
        gfx_draw_string_centered(fb, cx, card_y + 210, "1. Place the DualShock 4 on a flat, stable surface.", COLOR_TEXT_MUTED, 1);
        gfx_draw_string_centered(fb, cx, card_y + 250, "2. Ensure both analog sticks are completely untouched.", COLOR_TEXT_MUTED, 1);
        gfx_draw_string_centered(fb, cx, card_y + 290, "3. Press [X] to begin 120-frame mathematical sampling.", COLOR_TEXT_MUTED, 1);
        
        int btn_w = 320;
        int btn_h = 60;
        int btn_x = (SCREEN_WIDTH - btn_w) / 2;
        int btn_y = card_y + 380;
        gfx_fill_rounded_rect(fb, btn_x, btn_y, btn_w, btn_h, 8, COLOR_BTN_ACTIVE);
        gfx_draw_rounded_rect(fb, btn_x, btn_y, btn_w, btn_h, 8, COLOR_CYAN);
        gfx_draw_string_centered(fb, cx, btn_y + 20, "[X] START DRIFT TEST", COLOR_TEXT_WHITE, 1);
    }
}

/* Render Calibration Wizard */
static void render_calibration_wizard(Framebuffer* fb, const CalibrationWizard* calib, const DS4Controller* ctrl) {
    (void)ctrl;
    int cx = SCREEN_WIDTH / 2;
    int card_w = 1200;
    int card_h = 680;
    int card_x = (SCREEN_WIDTH - card_w) / 2;
    int card_y = 160;
    
    gfx_fill_rounded_rect(fb, card_x, card_y, card_w, card_h, 12, COLOR_PANEL);
    gfx_draw_rounded_rect(fb, card_x, card_y, card_w, card_h, 12, COLOR_BORDER);
    
    /* Hardware notice banner */
    gfx_fill_rounded_rect(fb, card_x + 40, card_y + 30, card_w - 80, 50, 6, COLOR_RGB(30, 40, 65));
    gfx_draw_string(fb, card_x + 60, card_y + 45, 
                    "NOTE: SOFTWARE CALIBRATION PROFILE ONLY (PS4 unprivileged userspace cannot reflash DS4 EEPROM)", 
                    COLOR_WARN, 1);
    
    if (!calib) return;
    
    char buf[128];
    switch (calib->current_step) {
        case CALIB_STEP_INTRO:
            gfx_draw_string_centered(fb, cx, card_y + 140, "CALIBRATION WIZARD: STEP 1 OF 5", COLOR_CYAN, 2);
            gfx_draw_string_centered(fb, cx, card_y + 220, "Release both analog sticks.", COLOR_TEXT_WHITE, 3);
            gfx_draw_string_centered(fb, cx, card_y + 300, "Ensure sticks rest naturally at center without any contact.", COLOR_TEXT_MUTED, 1);
            
            gfx_fill_rounded_rect(fb, cx - 150, card_y + 440, 300, 60, 8, COLOR_BTN_ACTIVE);
            gfx_draw_string_centered(fb, cx, card_y + 460, "[X] CONTINUE", COLOR_TEXT_WHITE, 1);
            break;
            
        case CALIB_STEP_LEFT_CENTER:
            gfx_draw_string_centered(fb, cx, card_y + 140, "CALIBRATION WIZARD: STEP 2 OF 5", COLOR_CYAN, 2);
            gfx_draw_string_centered(fb, cx, card_y + 220, "Keep LEFT stick centered.", COLOR_TEXT_WHITE, 3);
            gfx_draw_string_centered(fb, cx, card_y + 290, "Calculating neutral center offset...", COLOR_CYAN, 1);
            
            snprintf(buf, sizeof(buf), "Samples: %d / 90", calib->sample_counter);
            gfx_draw_string_centered(fb, cx, card_y + 360, buf, COLOR_TEXT_MUTED, 1);
            break;
            
        case CALIB_STEP_LEFT_RANGE:
            gfx_draw_string_centered(fb, cx, card_y + 140, "CALIBRATION WIZARD: STEP 3 OF 5", COLOR_CYAN, 2);
            gfx_draw_string_centered(fb, cx, card_y + 210, "Move LEFT stick through full range.", COLOR_TEXT_WHITE, 3);
            gfx_draw_string_centered(fb, cx, card_y + 270, "Rotate 360 degrees around outer boundary.", COLOR_TEXT_MUTED, 1);
            
            snprintf(buf, sizeof(buf), "Observed Limits: X [%d, %d]   Y [%d, %d]", 
                     calib->left.min_x, calib->left.max_x, calib->left.min_y, calib->left.max_y);
            gfx_draw_string_centered(fb, cx, card_y + 360, buf, COLOR_PASS, 1);
            
            gfx_fill_rounded_rect(fb, cx - 150, card_y + 460, 300, 60, 8, COLOR_BTN_ACTIVE);
            gfx_draw_string_centered(fb, cx, card_y + 480, "[X] NEXT STICK", COLOR_TEXT_WHITE, 1);
            break;
            
        case CALIB_STEP_RIGHT_CENTER:
            gfx_draw_string_centered(fb, cx, card_y + 140, "CALIBRATION WIZARD: STEP 4 OF 5", COLOR_CYAN, 2);
            gfx_draw_string_centered(fb, cx, card_y + 220, "Keep RIGHT stick centered.", COLOR_TEXT_WHITE, 3);
            gfx_draw_string_centered(fb, cx, card_y + 290, "Calculating neutral center offset...", COLOR_CYAN, 1);
            
            snprintf(buf, sizeof(buf), "Samples: %d / 90", calib->sample_counter);
            gfx_draw_string_centered(fb, cx, card_y + 360, buf, COLOR_TEXT_MUTED, 1);
            break;
            
        case CALIB_STEP_RIGHT_RANGE:
            gfx_draw_string_centered(fb, cx, card_y + 140, "CALIBRATION WIZARD: STEP 5 OF 5", COLOR_CYAN, 2);
            gfx_draw_string_centered(fb, cx, card_y + 210, "Move RIGHT stick through full range.", COLOR_TEXT_WHITE, 3);
            gfx_draw_string_centered(fb, cx, card_y + 270, "Rotate 360 degrees around outer boundary.", COLOR_TEXT_MUTED, 1);
            
            snprintf(buf, sizeof(buf), "Observed Limits: X [%d, %d]   Y [%d, %d]", 
                     calib->right.min_x, calib->right.max_x, calib->right.min_y, calib->right.max_y);
            gfx_draw_string_centered(fb, cx, card_y + 360, buf, COLOR_PASS, 1);
            
            gfx_fill_rounded_rect(fb, cx - 150, card_y + 460, 300, 60, 8, COLOR_BTN_ACTIVE);
            gfx_draw_string_centered(fb, cx, card_y + 480, "[X] GENERATE PROFILE", COLOR_TEXT_WHITE, 1);
            break;
            
        case CALIB_STEP_RESULT:
            gfx_draw_string_centered(fb, cx, card_y + 120, "CALIBRATION RESULT", COLOR_PASS, 3);
            
            int col1 = card_x + 150;
            int col2 = card_x + 680;
            int row_y = card_y + 200;
            
            gfx_draw_string(fb, col1, row_y, "LEFT STICK PROFILE", COLOR_CYAN, 2);
            snprintf(buf, sizeof(buf), "Center Offset:  X = %.1f, Y = %.1f", calib->left.center_x, calib->left.center_y);
            gfx_draw_string(fb, col1, row_y + 50, buf, COLOR_TEXT_WHITE, 1);
            snprintf(buf, sizeof(buf), "Range Bounds:   X [%d - %d], Y [%d - %d]", 
                     calib->left.min_x, calib->left.max_x, calib->left.min_y, calib->left.max_y);
            gfx_draw_string(fb, col1, row_y + 90, buf, COLOR_TEXT_WHITE, 1);
            snprintf(buf, sizeof(buf), "Deadzone:       %.3f", calib->left.deadzone);
            gfx_draw_string(fb, col1, row_y + 130, buf, COLOR_WARN, 1);
            
            gfx_draw_string(fb, col2, row_y, "RIGHT STICK PROFILE", COLOR_PASS, 2);
            snprintf(buf, sizeof(buf), "Center Offset:  X = %.1f, Y = %.1f", calib->right.center_x, calib->right.center_y);
            gfx_draw_string(fb, col2, row_y + 50, buf, COLOR_TEXT_WHITE, 1);
            snprintf(buf, sizeof(buf), "Range Bounds:   X [%d - %d], Y [%d - %d]", 
                     calib->right.min_x, calib->right.max_x, calib->right.min_y, calib->right.max_y);
            gfx_draw_string(fb, col2, row_y + 90, buf, COLOR_TEXT_WHITE, 1);
            snprintf(buf, sizeof(buf), "Deadzone:       %.3f", calib->right.deadzone);
            gfx_draw_string(fb, col2, row_y + 130, buf, COLOR_WARN, 1);
            
            gfx_fill_rounded_rect(fb, cx - 200, card_y + 480, 400, 60, 8, COLOR_BTN_ACTIVE);
            gfx_draw_string_centered(fb, cx, card_y + 500, "[X] SAVE PROFILE TO DISK", COLOR_TEXT_WHITE, 1);
            break;
    }
}

/* Render Vibration Test */
static void render_vibration_test(Framebuffer* fb, const VibrationState* vib) {
    int cx = SCREEN_WIDTH / 2;
    int card_w = 1100;
    int card_h = 650;
    int card_x = (SCREEN_WIDTH - card_w) / 2;
    int card_y = 180;
    
    gfx_fill_rounded_rect(fb, card_x, card_y, card_w, card_h, 12, COLOR_PANEL);
    gfx_draw_rounded_rect(fb, card_x, card_y, card_w, card_h, 12, COLOR_BORDER);
    
    gfx_draw_string_centered(fb, cx, card_y + 40, "DUALSHOCK 4 VIBRATION TEST", COLOR_CYAN, 3);
    
    char buf[64];
    snprintf(buf, sizeof(buf), "VIBRATION API: %s", 
             vib ? (vib->api_status == API_STATUS_REAL ? "REAL (ACTIVE)" : "PARTIAL / EMULATED") : "NOT SUPPORTED");
    gfx_draw_string_centered(fb, cx, card_y + 100, buf, COLOR_PASS, 1);
    
    /* Motor Buttons */
    int btn_w = 260;
    int btn_h = 70;
    int btn_y = card_y + 200;
    
    /* Left Motor */
    gfx_fill_rounded_rect(fb, card_x + 80, btn_y, btn_w, btn_h, 8, COLOR_PANEL_ALT);
    gfx_draw_rounded_rect(fb, card_x + 80, btn_y, btn_w, btn_h, 8, COLOR_CYAN);
    gfx_draw_string_centered(fb, card_x + 80 + btn_w/2, btn_y + 15, "Left Motor (Heavy)", COLOR_TEXT_WHITE, 1);
    gfx_draw_string_centered(fb, card_x + 80 + btn_w/2, btn_y + 40, "[Square] Test", COLOR_CYAN, 1);
    
    /* Right Motor */
    gfx_fill_rounded_rect(fb, card_x + 420, btn_y, btn_w, btn_h, 8, COLOR_PANEL_ALT);
    gfx_draw_rounded_rect(fb, card_x + 420, btn_y, btn_w, btn_h, 8, COLOR_CYAN);
    gfx_draw_string_centered(fb, card_x + 420 + btn_w/2, btn_y + 15, "Right Motor (Light)", COLOR_TEXT_WHITE, 1);
    gfx_draw_string_centered(fb, card_x + 420 + btn_w/2, btn_y + 40, "[Triangle] Test", COLOR_CYAN, 1);
    
    /* Both Motors */
    gfx_fill_rounded_rect(fb, card_x + 760, btn_y, btn_w, btn_h, 8, COLOR_PANEL_ALT);
    gfx_draw_rounded_rect(fb, card_x + 760, btn_y, btn_w, btn_h, 8, COLOR_CYAN);
    gfx_draw_string_centered(fb, card_x + 760 + btn_w/2, btn_y + 15, "Both Motors", COLOR_TEXT_WHITE, 1);
    gfx_draw_string_centered(fb, card_x + 760 + btn_w/2, btn_y + 40, "[Cross] Test", COLOR_CYAN, 1);
    
    /* Intensity Bar */
    int bar_y = card_y + 360;
    gfx_draw_string_centered(fb, cx, bar_y, "RUMBLE INTENSITY LEVEL", COLOR_TEXT_WHITE, 2);
    
    int bar_w = 600;
    int bar_h = 24;
    int bar_x = (SCREEN_WIDTH - bar_w) / 2;
    gfx_fill_rect(fb, bar_x, bar_y + 40, bar_w, bar_h, COLOR_PANEL_ALT);
    gfx_draw_rect(fb, bar_x, bar_y + 40, bar_w, bar_h, COLOR_BORDER);
    
    int int_val = vib ? vib->intensity_pct : 75;
    int fill_w = (int)(((float)int_val / 100.0f) * bar_w);
    gfx_fill_rect(fb, bar_x, bar_y + 40, fill_w, bar_h, COLOR_CYAN);
    
    snprintf(buf, sizeof(buf), "Intensity: %d%%  ([L1] Decrease  |  [R1] Increase)", int_val);
    gfx_draw_string_centered(fb, cx, bar_y + 80, buf, COLOR_CYAN, 1);
    
    if (vib && vib->is_active) {
        gfx_draw_string_centered(fb, cx, card_y + 540, "VIBRATING ACTIVE...", COLOR_PASS, 2);
    }
}

/* Render Touchpad Test */
static void render_touchpad_test(Framebuffer* fb, const TouchpadState* touch) {
    int cx = SCREEN_WIDTH / 2;
    int card_w = 1200;
    int card_h = 680;
    int card_x = (SCREEN_WIDTH - card_w) / 2;
    int card_y = 160;
    
    gfx_fill_rounded_rect(fb, card_x, card_y, card_w, card_h, 12, COLOR_PANEL);
    gfx_draw_rounded_rect(fb, card_x, card_y, card_w, card_h, 12, COLOR_BORDER);
    
    gfx_draw_string_centered(fb, cx, card_y + 35, "TOUCHPAD HARDWARE TEST", COLOR_CYAN, 3);
    gfx_draw_string_centered(fb, cx, card_y + 75, "Touch and slide across the DualShock 4 capacitive surface", COLOR_TEXT_MUTED, 1);
    
    /* Touchpad Surface Canvas */
    int pad_w = 960;
    int pad_h = 420;
    int pad_x = (SCREEN_WIDTH - pad_w) / 2;
    int pad_y = card_y + 110;
    
    gfx_fill_rounded_rect(fb, pad_x, pad_y, pad_w, pad_h, 16, COLOR_PANEL_ALT);
    gfx_draw_rounded_rect(fb, pad_x, pad_y, pad_w, pad_h, 16, touch && touch->touch_active ? COLOR_CYAN : COLOR_BORDER);
    
    /* Subtle grid */
    for (int gx = pad_x + 80; gx < pad_x + pad_w; gx += 80) {
        gfx_draw_line(fb, gx, pad_y, gx, pad_y + pad_h, COLOR_RGB(32, 45, 68));
    }
    for (int gy = pad_y + 70; gy < pad_y + pad_h; gy += 70) {
        gfx_draw_line(fb, pad_x, gy, pad_x + pad_w, gy, COLOR_RGB(32, 45, 68));
    }
    
    /* Trail */
    if (touch && touch->trail_count > 0) {
        for (int i = 0; i < touch->trail_count; i++) {
            int tx = pad_x + (int)(((float)touch->trail_x[i] / 1920.0f) * pad_w);
            int ty = pad_y + (int)(((float)touch->trail_y[i] / 941.0f) * pad_h);
            gfx_fill_circle(fb, tx, ty, 5, COLOR_RGB(0, 180, 220));
        }
    }
    
    /* Current finger dot */
    if (touch && touch->touch_active) {
        int fx = pad_x + (int)(((float)touch->current_x / 1920.0f) * pad_w);
        int fy = pad_y + (int)(((float)touch->current_y / 941.0f) * pad_h);
        gfx_fill_circle(fb, fx, fy, 16, COLOR_CYAN);
        gfx_draw_circle(fb, fx, fy, 24, COLOR_PASS);
    }
    
    /* Status indicators */
    char buf[128];
    int st_y = card_y + 560;
    snprintf(buf, sizeof(buf), "Touch Coordinates: X = %4d, Y = %4d (DS4 Native: 1920x941)", 
             touch ? touch->current_x : 0, touch ? touch->current_y : 0);
    gfx_draw_string(fb, card_x + 80, st_y, buf, COLOR_TEXT_WHITE, 1);
    
    snprintf(buf, sizeof(buf), "Touch Contact: [%s]", touch && touch->touch_active ? "YES" : "NO");
    gfx_draw_string(fb, card_x + 80, st_y + 35, buf, touch && touch->touch_active ? COLOR_PASS : COLOR_TEXT_MUTED, 1);
    
    snprintf(buf, sizeof(buf), "Physical Click: [%s]", touch && touch->click_active ? "YES" : "NO");
    gfx_draw_string(fb, card_x + 500, st_y + 35, buf, touch && touch->click_active ? COLOR_PASS : COLOR_TEXT_MUTED, 1);
}

/* Render Controller Information */
static void render_controller_info(Framebuffer* fb, const DS4Controller* ctrl) {
    ControllerHidInfo info;
    hid_info_query(ctrl, &info);
    
    int card_w = 1200;
    int card_h = 760;
    int card_x = (SCREEN_WIDTH - card_w) / 2;
    int card_y = 120;
    
    gfx_fill_rounded_rect(fb, card_x, card_y, card_w, card_h, 12, COLOR_PANEL);
    gfx_draw_rounded_rect(fb, card_x, card_y, card_w, card_h, 12, COLOR_BORDER);
    
    gfx_draw_string(fb, card_x + 60, card_y + 40, "CONTROLLER HARDWARE INFORMATION", COLOR_CYAN, 2);
    gfx_fill_rect(fb, card_x + 60, card_y + 80, card_w - 120, 1, COLOR_BORDER);
    
    int left_col = card_x + 60;
    int y = card_y + 110;
    int row_h = 45;
    
    char buf[256];
    
    snprintf(buf, sizeof(buf), "Hardware Model:       %s", info.model);
    gfx_draw_string(fb, left_col, y, buf, COLOR_TEXT_WHITE, 1);
    
    snprintf(buf, sizeof(buf), "Vendor ID (VID):      %s", info.vid_str);
    gfx_draw_string(fb, left_col, y + row_h, buf, COLOR_TEXT_WHITE, 1);
    
    snprintf(buf, sizeof(buf), "Product ID (PID):     %s", info.pid_str);
    gfx_draw_string(fb, left_col, y + row_h * 2, buf, COLOR_TEXT_WHITE, 1);
    
    snprintf(buf, sizeof(buf), "Active Connection:    %s", info.connection);
    gfx_draw_string(fb, left_col, y + row_h * 3, buf, COLOR_PASS, 1);
    
    snprintf(buf, sizeof(buf), "USB Interface:        %s", info.usb_status);
    gfx_draw_string(fb, left_col, y + row_h * 4, buf, COLOR_TEXT_WHITE, 1);
    
    snprintf(buf, sizeof(buf), "Bluetooth Interface:  %s", info.bt_status);
    gfx_draw_string(fb, left_col, y + row_h * 5, buf, COLOR_TEXT_WHITE, 1);
    
    snprintf(buf, sizeof(buf), "Battery Level:        %s", info.battery);
    gfx_draw_string(fb, left_col, y + row_h * 6, buf, COLOR_CYAN, 1);
    
    snprintf(buf, sizeof(buf), "Firmware Revision:    %s", info.firmware);
    gfx_draw_string(fb, left_col, y + row_h * 7, buf, COLOR_WARN, 1);
    
    snprintf(buf, sizeof(buf), "Hardware Serial No:   %s", info.serial);
    gfx_draw_string(fb, left_col, y + row_h * 8, buf, COLOR_WARN, 1);
    
    /* Unsupported fields explanation */
    gfx_fill_rect(fb, card_x + 60, y + row_h * 9 + 10, card_w - 120, 1, COLOR_BORDER);
    gfx_draw_string(fb, left_col, y + row_h * 10, "UNSUPPORTED OR RESTRICTED PS4 SDK FIELDS:", COLOR_WARN, 1);
    gfx_draw_string(fb, left_col, y + row_h * 11, info.unsupported_fields, COLOR_TEXT_MUTED, 1);
}

/* Render Diagnostic Report */
static void render_diagnostic_report(Framebuffer* fb, const DS4Controller* ctrl, const DiagnosticSession* diag) {
    int card_w = 1200;
    int card_h = 760;
    int card_x = (SCREEN_WIDTH - card_w) / 2;
    int card_y = 120;
    
    gfx_fill_rounded_rect(fb, card_x, card_y, card_w, card_h, 12, COLOR_PANEL);
    gfx_draw_rounded_rect(fb, card_x, card_y, card_w, card_h, 12, COLOR_BORDER);
    
    gfx_draw_string(fb, card_x + 60, card_y + 40, "HARDWARE DIAGNOSTIC REPORT", COLOR_CYAN, 2);
    gfx_fill_rect(fb, card_x + 60, card_y + 80, card_w - 120, 1, COLOR_BORDER);
    
    int row_y = card_y + 110;
    int h = 48;
    
    struct {
        const char* name;
        const char* status;
        uint32_t color;
    } items[] = {
        {"CONTROLLER DETECTION", ctrl && ctrl->connected ? "CONNECTED" : "FAILED", ctrl && ctrl->connected ? COLOR_PASS : COLOR_FAIL},
        {"BUTTON MATRIX TEST",   diagnostics_result_to_string(diag ? diag->buttons_result : RESULT_NOT_TESTED), COLOR_PASS},
        {"LEFT ANALOG STICK",    diagnostics_result_to_string(diag ? diag->left_stick_result : RESULT_NOT_TESTED), COLOR_PASS},
        {"RIGHT ANALOG STICK",   diagnostics_result_to_string(diag ? diag->right_stick_result : RESULT_NOT_TESTED), COLOR_PASS},
        {"LEFT DRIFT STATUS",    diag && diag->left_drift_detected ? "DRIFT DETECTED" : "PASS", diag && diag->left_drift_detected ? COLOR_FAIL : COLOR_PASS},
        {"RIGHT DRIFT STATUS",   diag && diag->right_drift_detected ? "DRIFT DETECTED" : "PASS", diag && diag->right_drift_detected ? COLOR_FAIL : COLOR_PASS},
        {"TOUCHPAD INTERFACE",   diagnostics_result_to_string(diag ? diag->touchpad_result : RESULT_NOT_TESTED), COLOR_PASS},
        {"VIBRATION MOTORS",     diagnostics_result_to_string(diag ? diag->vibration_result : RESULT_NOT_TESTED), COLOR_PASS},
        {"OVERALL EVALUATION",   diagnostics_result_to_string(diag ? diag->overall_result : RESULT_PASS), COLOR_PASS},
    };
    
    for (size_t i = 0; i < sizeof(items)/sizeof(items[0]); i++) {
        int y = row_y + (int)i * h;
        gfx_draw_string(fb, card_x + 80, y, items[i].name, COLOR_TEXT_WHITE, 1);
        gfx_draw_string(fb, card_x + 700, y, items[i].status, items[i].color, 1);
    }
    
    /* Save report button */
    int btn_w = 400;
    int btn_h = 55;
    int btn_x = (SCREEN_WIDTH - btn_w) / 2;
    int btn_y = card_y + 640;
    gfx_fill_rounded_rect(fb, btn_x, btn_y, btn_w, btn_h, 8, COLOR_BTN_ACTIVE);
    gfx_draw_rounded_rect(fb, btn_x, btn_y, btn_w, btn_h, 8, COLOR_CYAN);
    gfx_draw_string_centered(fb, SCREEN_WIDTH / 2, btn_y + 18, "[X] EXPORT REPORT TO /DATA", COLOR_TEXT_WHITE, 1);
}

void ui_render(UIState* ui, Framebuffer* fb, const DS4Controller* ctrl, 
               const JoystickAnalyzer* ja, const CalibrationWizard* calib, 
               const VibrationState* vib, const TouchpadState* touch, 
               const DiagnosticSession* diag, const AppSettings* settings) {
    (void)settings;
    if (!ui || !fb) return;
    
    gfx_clear(fb, COLOR_BG);
    ui->frame_count++;
    
    /* Header */
    const char* view_titles[] = {
        "MAIN MENU",
        "CONTROLLER BUTTON TEST",
        "JOYSTICK ANALYZER",
        "DRIFT DETECTOR",
        "RANGE SWEEP TEST",
        "CALIBRATION WIZARD",
        "DEADZONE ANALYZER",
        "VIBRATION TEST",
        "TOUCHPAD TEST",
        "CONTROLLER INFORMATION",
        "DIAGNOSTIC REPORT",
        "FULL DIAGNOSTIC SESSION",
        "SETTINGS",
        "ABOUT DEVELOPER",
        "DISCONNECTED"
    };
    draw_header(fb, view_titles[ui->current_view], ctrl);
    
    /* View-specific drawing */
    switch (ui->current_view) {
        case VIEW_MAIN_MENU:
            render_main_menu(fb, ui, ctrl);
            break;
        case VIEW_CONTROLLER_TEST:
            render_controller_test(fb, ctrl);
            break;
        case VIEW_JOYSTICK_ANALYZER:
            render_joystick_analyzer(fb, ctrl, ja);
            break;
        case VIEW_DRIFT_DETECTOR:
            render_drift_detector(fb, ja);
            break;
        case VIEW_CALIBRATION_WIZARD:
            render_calibration_wizard(fb, calib, ctrl);
            break;
        case VIEW_VIBRATION_TEST:
            render_vibration_test(fb, vib);
            break;
        case VIEW_TOUCHPAD_TEST:
            render_touchpad_test(fb, touch);
            break;
        case VIEW_CONTROLLER_INFO:
            render_controller_info(fb, ctrl);
            break;
        case VIEW_DIAGNOSTIC_REPORT:
            render_diagnostic_report(fb, ctrl, diag);
            break;
        default:
            render_main_menu(fb, ui, ctrl);
            break;
    }
    
    /* Toast overlay */
    draw_toast(fb, ui);
    
    /* Footer */
    draw_footer(fb, ui);
}

void ui_handle_input(UIState* ui, DS4Controller* ctrl, JoystickAnalyzer* ja, 
                     CalibrationWizard* calib, VibrationState* vib, 
                     TouchpadState* touch, DiagnosticSession* diag, 
                     AppSettings* settings) {
    (void)touch;
    (void)diag;
    if (!ui || !ctrl) return;
    
    /* Universal Back Navigation (Circle = Back) */
    if (controller_button_down(ctrl, DS4_BTN_CIRCLE)) {
        if (ui->current_view != VIEW_MAIN_MENU) {
            ui->current_view = VIEW_MAIN_MENU;
            return;
        }
    }
    
    /* View-specific input */
    switch (ui->current_view) {
        case VIEW_MAIN_MENU:
            if (controller_button_down(ctrl, DS4_BTN_UP)) {
                ui->menu_cursor = (ui->menu_cursor - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
            } else if (controller_button_down(ctrl, DS4_BTN_DOWN)) {
                ui->menu_cursor = (ui->menu_cursor + 1) % MENU_ITEM_COUNT;
            } else if (controller_button_down(ctrl, DS4_BTN_CROSS)) {
                switch (ui->menu_cursor) {
                    case 0: ui->current_view = VIEW_CONTROLLER_TEST; break;
                    case 1: ui->current_view = VIEW_JOYSTICK_ANALYZER; break;
                    case 2: ui->current_view = VIEW_CALIBRATION_WIZARD; calibration_reset(calib); break;
                    case 3: ui->current_view = VIEW_VIBRATION_TEST; break;
                    case 4: ui->current_view = VIEW_TOUCHPAD_TEST; break;
                    case 5: ui->current_view = VIEW_CONTROLLER_INFO; break;
                    case 6: ui->current_view = VIEW_DIAGNOSTIC_REPORT; break;
                    case 7: ui->current_view = VIEW_SETTINGS; break;
                    case 8: /* Exit */ break;
                }
            }
            break;
            
        case VIEW_CONTROLLER_TEST:
            if (controller_button_down(ctrl, DS4_BTN_CROSS)) {
                controller_reset_tested_buttons(ctrl);
                ui_show_toast(ui, "Button Matrix Reset", 90);
            }
            break;
            
        case VIEW_JOYSTICK_ANALYZER:
            if (controller_button_down(ctrl, DS4_BTN_SQUARE)) {
                ui->current_view = VIEW_DRIFT_DETECTOR;
            } else if (controller_button_down(ctrl, DS4_BTN_TRIANGLE)) {
                ui->current_view = VIEW_CALIBRATION_WIZARD;
                calibration_reset(calib);
            }
            break;
            
        case VIEW_DRIFT_DETECTOR:
            if (controller_button_down(ctrl, DS4_BTN_CROSS)) {
                joystick_start_drift_test(ja, settings ? settings->deadzone_threshold : 0.08f);
                ui_show_toast(ui, "Sampling Drift...", 60);
            }
            break;
            
        case VIEW_CALIBRATION_WIZARD:
            if (controller_button_down(ctrl, DS4_BTN_CROSS)) {
                if (calib->current_step == CALIB_STEP_RESULT) {
                    calibration_save_profile(calib, BASE_DATA_DIR "/profiles/calibration.json");
                    ui_show_toast(ui, "Software Profile Saved to /data", 120);
                } else {
                    calibration_advance(calib);
                }
            }
            break;
            
        case VIEW_VIBRATION_TEST:
            if (controller_button_down(ctrl, DS4_BTN_SQUARE)) {
                vibration_trigger(vib, ctrl->handle, VIB_TARGET_LEFT, vib->intensity_pct, 45);
            } else if (controller_button_down(ctrl, DS4_BTN_TRIANGLE)) {
                vibration_trigger(vib, ctrl->handle, VIB_TARGET_RIGHT, vib->intensity_pct, 45);
            } else if (controller_button_down(ctrl, DS4_BTN_CROSS)) {
                vibration_trigger(vib, ctrl->handle, VIB_TARGET_BOTH, vib->intensity_pct, 60);
            } else if (controller_button_down(ctrl, DS4_BTN_L1)) {
                if (vib->intensity_pct >= 10) vib->intensity_pct -= 10;
            } else if (controller_button_down(ctrl, DS4_BTN_R1)) {
                if (vib->intensity_pct <= 90) vib->intensity_pct += 10;
            }
            break;
            
        case VIEW_DIAGNOSTIC_REPORT:
            if (controller_button_down(ctrl, DS4_BTN_CROSS)) {
                char json_data[2048];
                char txt_data[2048];
                report_generate_json(ctrl, diag, json_data, sizeof(json_data));
                report_generate_text(ctrl, diag, txt_data, sizeof(txt_data));
                report_save_to_disk(BASE_DATA_DIR "/reports", json_data, txt_data);
                ui_show_toast(ui, "Diagnostic Report Exported to /data/reports/", 150);
            }
            break;
            
        default:
            break;
    }
}
