#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

// Color type (32-bit ARGB)
typedef uint32_t color_t;

// Common colors
#define COLOR_BLACK       0x00000000
#define COLOR_WHITE       0x00FFFFFF
#define COLOR_RED         0x00FF0000
#define COLOR_GREEN       0x0000FF00
#define COLOR_BLUE        0x000000FF
#define COLOR_GRAY        0x00808080
#define COLOR_LIGHT_GRAY  0x00C0C0C0
#define COLOR_DARK_GRAY   0x00404040
#define COLOR_CYAN        0x0000FFFF
#define COLOR_YELLOW      0x00FFFF00

// Make RGB color
#define RGB(r, g, b) ((color_t)(((r) << 16) | ((g) << 8) | (b)))

// Graphics info structure
typedef struct {
    uint32_t* framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;      // bytes per row
    uint8_t bpp;         // bits per pixel
    int initialized;
} graphics_info_t;

// Global graphics info - defined in graphics.c
extern graphics_info_t g_graphics;

// Graphics initialization functions
void graphics_init(void* multiboot_info);
int graphics_is_available(void);
uint32_t graphics_get_width(void);
uint32_t graphics_get_height(void);
void graphics_swap_buffers(void);

// Drawing functions
void draw_pixel(int x, int y, color_t color);
void draw_rect(int x, int y, int width, int height, color_t color);
void draw_filled_rect(int x, int y, int width, int height, color_t color);
void draw_line(int x1, int y1, int x2, int y2, color_t color);
void draw_hline(int x, int y, int width, color_t color);
void draw_vline(int x, int y, int height, color_t color);
void clear_screen(color_t color);

#endif
