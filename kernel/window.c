#include "window.h"
#include "graphics.h"
#include "font.h"
#include "string.h"

// Colors for window decorations
#define COLOR_TITLEBAR_FOCUSED   RGB(0, 0, 128)    // Dark blue (#000080)
#define COLOR_TITLEBAR_UNFOCUSED RGB(128, 128, 128) // Gray (#808080)
#define COLOR_TITLEBAR_TEXT      COLOR_WHITE
#define COLOR_WINDOW_BG          RGB(192, 192, 192) // Light gray (#C0C0C0)
#define COLOR_WINDOW_BORDER      COLOR_DARK_GRAY

// Close button dimensions
#define CLOSE_BTN_SIZE 16
#define CLOSE_BTN_MARGIN 4

// Window array and count
static window_t windows[MAX_WINDOWS];
static int window_count = 0;

// Z-order array (indices into windows array, front to back)
static int z_order[MAX_WINDOWS];
static int z_count = 0;

// Initialize window manager
void wm_init(void) {
    window_count = 0;
    z_count = 0;

    // Clear all windows
    for (int i = 0; i < MAX_WINDOWS; i++) {
        windows[i].visible = 0;
        windows[i].focused = 0;
        windows[i].draw_content = 0;
        windows[i].on_key = 0;
    }
}

// Find an empty window slot
static int find_free_slot(void) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!windows[i].visible) {
            return i;
        }
    }
    return -1;
}

// Create a new window
window_t* wm_create_window(int x, int y, int width, int height, const char* title) {
    int slot = find_free_slot();
    if (slot < 0) {
        return 0;  // No free slots
    }

    window_t* win = &windows[slot];

    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->visible = 1;
    win->focused = 0;
    win->bg_color = COLOR_WINDOW_BG;
    win->draw_content = 0;
    win->on_key = 0;

    // Copy title
    int i = 0;
    while (title[i] && i < 63) {
        win->title[i] = title[i];
        i++;
    }
    win->title[i] = '\0';

    // Add to z-order (at front)
    z_order[z_count] = slot;
    z_count++;

    // Focus the new window
    wm_focus_window(win);

    return win;
}

// Destroy a window
void wm_destroy_window(window_t* win) {
    if (!win || !win->visible) return;

    // Find window index
    int win_idx = -1;
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (&windows[i] == win) {
            win_idx = i;
            break;
        }
    }

    if (win_idx < 0) return;

    // Remove from z-order
    int z_idx = -1;
    for (int i = 0; i < z_count; i++) {
        if (z_order[i] == win_idx) {
            z_idx = i;
            break;
        }
    }

    if (z_idx >= 0) {
        // Shift z-order array
        for (int i = z_idx; i < z_count - 1; i++) {
            z_order[i] = z_order[i + 1];
        }
        z_count--;
    }

    // Clear window
    win->visible = 0;
    win->focused = 0;

    // Focus top window if any
    if (z_count > 0) {
        wm_focus_window(&windows[z_order[z_count - 1]]);
    }
}

// Focus a window (bring to front)
void wm_focus_window(window_t* win) {
    if (!win || !win->visible) return;

    // Find window index
    int win_idx = -1;
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (&windows[i] == win) {
            win_idx = i;
            break;
        }
    }

    if (win_idx < 0) return;

    // Unfocus all windows
    for (int i = 0; i < MAX_WINDOWS; i++) {
        windows[i].focused = 0;
    }

    // Focus this window
    win->focused = 1;

    // Move to front of z-order
    int z_idx = -1;
    for (int i = 0; i < z_count; i++) {
        if (z_order[i] == win_idx) {
            z_idx = i;
            break;
        }
    }

    if (z_idx >= 0 && z_idx < z_count - 1) {
        // Shift to bring to front
        for (int i = z_idx; i < z_count - 1; i++) {
            z_order[i] = z_order[i + 1];
        }
        z_order[z_count - 1] = win_idx;
    }
}

// Get focused window
window_t* wm_get_focused(void) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].visible && windows[i].focused) {
            return &windows[i];
        }
    }
    return 0;
}

// Get content area coordinates
int wm_content_x(window_t* win) {
    return win->x + WINDOW_BORDER;
}

int wm_content_y(window_t* win) {
    return win->y + TITLEBAR_HEIGHT + WINDOW_BORDER;
}

int wm_content_width(window_t* win) {
    return win->width - (2 * WINDOW_BORDER);
}

int wm_content_height(window_t* win) {
    return win->height - TITLEBAR_HEIGHT - (2 * WINDOW_BORDER);
}

// Draw window frame/border
void wm_draw_frame(window_t* win) {
    if (!win || !win->visible) return;

    // Draw outer border
    draw_rect(win->x, win->y, win->width, win->height, COLOR_WINDOW_BORDER);

    // Draw inner border (1 pixel inside)
    draw_rect(win->x + 1, win->y + 1, win->width - 2, win->height - 2, COLOR_WHITE);
}

// Draw window titlebar
void wm_draw_titlebar(window_t* win) {
    if (!win || !win->visible) return;

    color_t titlebar_color = win->focused ? COLOR_TITLEBAR_FOCUSED : COLOR_TITLEBAR_UNFOCUSED;

    // Draw titlebar background
    draw_filled_rect(win->x + WINDOW_BORDER,
                     win->y + WINDOW_BORDER,
                     win->width - (2 * WINDOW_BORDER),
                     TITLEBAR_HEIGHT - WINDOW_BORDER,
                     titlebar_color);

    // Draw title text (centered vertically in titlebar)
    int text_y = win->y + WINDOW_BORDER + (TITLEBAR_HEIGHT - WINDOW_BORDER - FONT_HEIGHT) / 2;
    int text_x = win->x + WINDOW_BORDER + 4;
    font_draw_string(text_x, text_y, win->title, COLOR_TITLEBAR_TEXT, titlebar_color);

    // Draw close button (red X in top-right corner)
    int btn_x = win->x + win->width - WINDOW_BORDER - CLOSE_BTN_SIZE - CLOSE_BTN_MARGIN;
    int btn_y = win->y + WINDOW_BORDER + (TITLEBAR_HEIGHT - WINDOW_BORDER - CLOSE_BTN_SIZE) / 2;

    // Button background
    draw_filled_rect(btn_x, btn_y, CLOSE_BTN_SIZE, CLOSE_BTN_SIZE, COLOR_LIGHT_GRAY);
    draw_rect(btn_x, btn_y, CLOSE_BTN_SIZE, CLOSE_BTN_SIZE, COLOR_DARK_GRAY);

    // Draw X
    color_t x_color = COLOR_BLACK;
    for (int i = 2; i < CLOSE_BTN_SIZE - 2; i++) {
        draw_pixel(btn_x + i, btn_y + i, x_color);
        draw_pixel(btn_x + i + 1, btn_y + i, x_color);
        draw_pixel(btn_x + CLOSE_BTN_SIZE - 1 - i, btn_y + i, x_color);
        draw_pixel(btn_x + CLOSE_BTN_SIZE - 2 - i, btn_y + i, x_color);
    }
}

// Draw a single window
void wm_draw_window(window_t* win) {
    if (!win || !win->visible) return;

    // Draw frame
    wm_draw_frame(win);

    // Draw titlebar
    wm_draw_titlebar(win);

    // Draw content area background
    int cx = wm_content_x(win);
    int cy = wm_content_y(win);
    int cw = wm_content_width(win);
    int ch = wm_content_height(win);

    draw_filled_rect(cx, cy, cw, ch, win->bg_color);

    // Call draw_content callback if set
    if (win->draw_content) {
        win->draw_content(win);
    }
}

// Draw all visible windows (back to front)
void wm_draw_all(void) {
    // Draw in z-order (back to front)
    for (int i = 0; i < z_count; i++) {
        window_t* win = &windows[z_order[i]];
        if (win->visible) {
            wm_draw_window(win);
        }
    }
}

// Check if point is inside close button
static int point_in_close_button(window_t* win, int mx, int my) {
    int btn_x = win->x + win->width - WINDOW_BORDER - CLOSE_BTN_SIZE - CLOSE_BTN_MARGIN;
    int btn_y = win->y + WINDOW_BORDER + (TITLEBAR_HEIGHT - WINDOW_BORDER - CLOSE_BTN_SIZE) / 2;

    return (mx >= btn_x && mx < btn_x + CLOSE_BTN_SIZE &&
            my >= btn_y && my < btn_y + CLOSE_BTN_SIZE);
}

// Check if point is inside titlebar (excluding close button)
static int point_in_titlebar(window_t* win, int mx, int my) {
    if (point_in_close_button(win, mx, my)) {
        return 0;
    }

    return (mx >= win->x + WINDOW_BORDER &&
            mx < win->x + win->width - WINDOW_BORDER &&
            my >= win->y + WINDOW_BORDER &&
            my < win->y + TITLEBAR_HEIGHT);
}

// Check if point is inside window
static int point_in_window(window_t* win, int mx, int my) {
    return (mx >= win->x && mx < win->x + win->width &&
            my >= win->y && my < win->y + win->height);
}

// Handle mouse input
void wm_handle_mouse(int x, int y, int buttons) {
    // Only process on button press (left button)
    if (!(buttons & 1)) return;

    // Check windows from front to back
    for (int i = z_count - 1; i >= 0; i--) {
        window_t* win = &windows[z_order[i]];
        if (!win->visible) continue;

        if (point_in_window(win, x, y)) {
            // Check close button first
            if (point_in_close_button(win, x, y)) {
                wm_destroy_window(win);
                return;
            }

            // Check titlebar for focus/drag
            if (point_in_titlebar(win, x, y)) {
                wm_focus_window(win);
                // Note: Drag handling would require tracking state
                return;
            }

            // Click in content area - just focus
            wm_focus_window(win);
            return;
        }
    }
}

// Handle keyboard input
void wm_handle_key(unsigned char key) {
    window_t* focused = wm_get_focused();
    if (focused && focused->on_key) {
        focused->on_key(focused, key);
    }
}
