#include "../include/graphics.h"

/* External reference to graphics info from graphics.c */
extern graphics_info_t g_graphics;

void draw_pixel(int x, int y, color_t color) {
    if (x < 0 || x >= (int)g_graphics.width || y < 0 || y >= (int)g_graphics.height)
        return;
    /* pitch is in bytes, we're writing 32-bit pixels */
    uint32_t* pixel = (uint32_t*)((uint8_t*)g_graphics.framebuffer + y * g_graphics.pitch + x * 4);
    *pixel = color;
}

void draw_filled_rect(int x, int y, int width, int height, color_t color) {
    for (int row = y; row < y + height; row++) {
        for (int col = x; col < x + width; col++) {
            draw_pixel(col, row, color);
        }
    }
}

void draw_rect(int x, int y, int width, int height, color_t color) {
    draw_hline(x, y, width, color);                    /* top */
    draw_hline(x, y + height - 1, width, color);       /* bottom */
    draw_vline(x, y, height, color);                   /* left */
    draw_vline(x + width - 1, y, height, color);       /* right */
}

void draw_hline(int x, int y, int width, color_t color) {
    for (int i = 0; i < width; i++) {
        draw_pixel(x + i, y, color);
    }
}

void draw_vline(int x, int y, int height, color_t color) {
    for (int i = 0; i < height; i++) {
        draw_pixel(x, y + i, color);
    }
}

void draw_line(int x1, int y1, int x2, int y2, color_t color) {
    /* Bresenham's line algorithm */
    int dx = x2 - x1;
    int dy = y2 - y1;
    int sx = (dx > 0) ? 1 : -1;
    int sy = (dy > 0) ? 1 : -1;
    dx = (dx < 0) ? -dx : dx;
    dy = (dy < 0) ? -dy : dy;

    int err = dx - dy;

    while (1) {
        draw_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

void clear_screen(color_t color) {
    draw_filled_rect(0, 0, g_graphics.width, g_graphics.height, color);
}
