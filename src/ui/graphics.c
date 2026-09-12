#include "graphics.h"
#include "font.h"
#include <stdlib.h>
#include <string.h>

void gfx_init(Framebuffer* fb, uint32_t* buffer, int width, int height) {
    if (!fb) return;
    fb->buffer = buffer;
    fb->width = width;
    fb->height = height;
    fb->pitch = width;
}

void gfx_clear(Framebuffer* fb, uint32_t color) {
    if (!fb || !fb->buffer) return;
    int total = fb->width * fb->height;
    for (int i = 0; i < total; i++) {
        fb->buffer[i] = color;
    }
}

void gfx_draw_pixel(Framebuffer* fb, int x, int y, uint32_t color) {
    if (!fb || !fb->buffer) return;
    if (x < 0 || x >= fb->width || y < 0 || y >= fb->height) return;
    fb->buffer[y * fb->pitch + x] = color;
}

void gfx_draw_line(Framebuffer* fb, int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    
    while (1) {
        gfx_draw_pixel(fb, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void gfx_draw_rect(Framebuffer* fb, int x, int y, int w, int h, uint32_t color) {
    gfx_draw_line(fb, x, y, x + w - 1, y, color);
    gfx_draw_line(fb, x, y + h - 1, x + w - 1, y + h - 1, color);
    gfx_draw_line(fb, x, y, x, y + h - 1, color);
    gfx_draw_line(fb, x + w - 1, y, x + w - 1, y + h - 1, color);
}

void gfx_fill_rect(Framebuffer* fb, int x, int y, int w, int h, uint32_t color) {
    if (!fb || !fb->buffer) return;
    int x_end = x + w;
    int y_end = y + h;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x_end > fb->width) x_end = fb->width;
    if (y_end > fb->height) y_end = fb->height;
    
    for (int py = y; py < y_end; py++) {
        uint32_t* row = &fb->buffer[py * fb->pitch + x];
        int count = x_end - x;
        for (int px = 0; px < count; px++) {
            row[px] = color;
        }
    }
}

void gfx_draw_rounded_rect(Framebuffer* fb, int x, int y, int w, int h, int r, uint32_t color) {
    if (r <= 0) {
        gfx_draw_rect(fb, x, y, w, h, color);
        return;
    }
    gfx_draw_line(fb, x + r, y, x + w - r, y, color);
    gfx_draw_line(fb, x + r, y + h, x + w - r, y + h, color);
    gfx_draw_line(fb, x, y + r, x, y + h - r, color);
    gfx_draw_line(fb, x + w, y + r, x + w, y + h - r, color);
}

void gfx_fill_rounded_rect(Framebuffer* fb, int x, int y, int w, int h, int r, uint32_t color) {
    if (r <= 0) {
        gfx_fill_rect(fb, x, y, w, h, color);
        return;
    }
    gfx_fill_rect(fb, x + r, y, w - 2 * r, h, color);
    gfx_fill_rect(fb, x, y + r, r, h - 2 * r, color);
    gfx_fill_rect(fb, x + w - r, y + r, r, h - 2 * r, color);
    gfx_fill_circle(fb, x + r, y + r, r, color);
    gfx_fill_circle(fb, x + w - r, y + r, r, color);
    gfx_fill_circle(fb, x + r, y + h - r, r, color);
    gfx_fill_circle(fb, x + w - r, y + h - r, r, color);
}

void gfx_draw_circle(Framebuffer* fb, int cx, int cy, int radius, uint32_t color) {
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        gfx_draw_pixel(fb, cx + x, cy + y, color);
        gfx_draw_pixel(fb, cx + y, cy + x, color);
        gfx_draw_pixel(fb, cx - y, cy + x, color);
        gfx_draw_pixel(fb, cx - x, cy + y, color);
        gfx_draw_pixel(fb, cx - x, cy - y, color);
        gfx_draw_pixel(fb, cx - y, cy - x, color);
        gfx_draw_pixel(fb, cx + y, cy - x, color);
        gfx_draw_pixel(fb, cx + x, cy - y, color);
        
        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

void gfx_fill_circle(Framebuffer* fb, int cx, int cy, int radius, uint32_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                gfx_draw_pixel(fb, cx + x, cy + y, color);
            }
        }
    }
}

void gfx_draw_char(Framebuffer* fb, int x, int y, char c, uint32_t color, int scale) {
    if (c < 32 || c > 126) c = '?';
    int glyph_idx = c - 32;
    if (scale <= 1) scale = 1;
    
    for (int row = 0; row < 16; row++) {
        uint8_t row_bits = g_font8x16[glyph_idx][row];
        for (int col = 0; col < 8; col++) {
            if (row_bits & (1 << (7 - col))) {
                if (scale == 1) {
                    gfx_draw_pixel(fb, x + col, y + row, color);
                } else {
                    gfx_fill_rect(fb, x + col * scale, y + row * scale, scale, scale, color);
                }
            }
        }
    }
}

void gfx_draw_string(Framebuffer* fb, int x, int y, const char* str, uint32_t color, int scale) {
    if (!str) return;
    if (scale <= 0) scale = 1;
    int cur_x = x;
    int cur_y = y;
    
    while (*str) {
        if (*str == '\n') {
            cur_x = x;
            cur_y += 18 * scale;
        } else {
            gfx_draw_char(fb, cur_x, cur_y, *str, color, scale);
            cur_x += 8 * scale;
        }
        str++;
    }
}

void gfx_draw_string_centered(Framebuffer* fb, int cx, int y, const char* str, uint32_t color, int scale) {
    if (!str) return;
    if (scale <= 0) scale = 1;
    int len = (int)strlen(str);
    int total_width = len * 8 * scale;
    int start_x = cx - (total_width / 2);
    gfx_draw_string(fb, start_x, y, str, color, scale);
}
