#ifndef ROGUEBYTE_GRAPHICS_H
#define ROGUEBYTE_GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>

#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080

/* Color Macros (ARGB 32-bit) */
#define COLOR_ARGB(a, r, g, b) (((uint32_t)(a) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))
#define COLOR_RGB(r, g, b)    COLOR_ARGB(255, r, g, b)

#define COLOR_BG          COLOR_RGB(11, 14, 20)      /* Dark Onyx */
#define COLOR_PANEL       COLOR_RGB(19, 27, 42)      /* Navy Slate */
#define COLOR_PANEL_ALT   COLOR_RGB(26, 36, 56)
#define COLOR_BORDER      COLOR_RGB(45, 62, 90)
#define COLOR_CYAN        COLOR_RGB(0, 210, 255)     /* Brand Accent */
#define COLOR_BLUE        COLOR_RGB(37, 99, 235)     /* Rogue Blue */
#define COLOR_PASS        COLOR_RGB(16, 185, 129)    /* Emerald Green */
#define COLOR_WARN        COLOR_RGB(245, 158, 11)    /* Amber */
#define COLOR_FAIL        COLOR_RGB(239, 68, 68)     /* Crimson */
#define COLOR_TEXT_WHITE  COLOR_RGB(248, 250, 252)
#define COLOR_TEXT_MUTED  COLOR_RGB(148, 163, 184)
#define COLOR_TEXT_DIM    COLOR_RGB(71, 85, 105)
#define COLOR_BTN_DEFAULT COLOR_RGB(30, 41, 59)
#define COLOR_BTN_ACTIVE  COLOR_RGB(37, 99, 235)

typedef struct {
    uint32_t* buffer;
    int width;
    int height;
    int pitch;
} Framebuffer;

void gfx_init(Framebuffer* fb, uint32_t* buffer, int width, int height);
void gfx_clear(Framebuffer* fb, uint32_t color);
void gfx_draw_pixel(Framebuffer* fb, int x, int y, uint32_t color);
void gfx_draw_line(Framebuffer* fb, int x0, int y0, int x1, int y1, uint32_t color);
void gfx_draw_rect(Framebuffer* fb, int x, int y, int w, int h, uint32_t color);
void gfx_fill_rect(Framebuffer* fb, int x, int y, int w, int h, uint32_t color);
void gfx_draw_rounded_rect(Framebuffer* fb, int x, int y, int w, int h, int r, uint32_t color);
void gfx_fill_rounded_rect(Framebuffer* fb, int x, int y, int w, int h, int r, uint32_t color);
void gfx_draw_circle(Framebuffer* fb, int cx, int cy, int radius, uint32_t color);
void gfx_fill_circle(Framebuffer* fb, int cx, int cy, int radius, uint32_t color);

void gfx_draw_char(Framebuffer* fb, int x, int y, char c, uint32_t color, int scale);
void gfx_draw_string(Framebuffer* fb, int x, int y, const char* str, uint32_t color, int scale);
void gfx_draw_string_centered(Framebuffer* fb, int cx, int y, const char* str, uint32_t color, int scale);

#endif /* ROGUEBYTE_GRAPHICS_H */
