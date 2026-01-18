#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>
#include "graphics.h"

#define MAX_WINDOWS 16
#define TITLEBAR_HEIGHT 24
#define WINDOW_BORDER 2

typedef struct window {
    int x, y;
    int width, height;
    char title[64];
    int visible;
    int focused;
    color_t bg_color;
    // Content buffer (optional - for now we'll draw directly)
    void (*draw_content)(struct window* win);
    void (*on_key)(struct window* win, char key);
} window_t;

// Window manager functions
void wm_init(void);
void wm_draw_all(void);
void wm_draw_window(window_t* win);
window_t* wm_create_window(int x, int y, int width, int height, const char* title);
void wm_destroy_window(window_t* win);
void wm_focus_window(window_t* win);
window_t* wm_get_focused(void);
void wm_handle_mouse(int x, int y, int buttons);
void wm_handle_key(char key);

// Draw window decorations
void wm_draw_titlebar(window_t* win);
void wm_draw_frame(window_t* win);

// Get content area (inside window chrome)
int wm_content_x(window_t* win);
int wm_content_y(window_t* win);
int wm_content_width(window_t* win);
int wm_content_height(window_t* win);

#endif
