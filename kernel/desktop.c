/*
 * AJOS Desktop Environment
 * Main desktop that ties together windows, taskbar, and mouse
 */

#include "desktop.h"
#include "graphics.h"
#include "window.h"
#include "taskbar.h"
#include "terminal.h"
#include "mouse.h"
#include "keyboard.h"
#include "font.h"

/* Mouse cursor dimensions */
#define CURSOR_WIDTH  12
#define CURSOR_HEIGHT 19

/* Mouse cursor bitmap (1 = white, 2 = black, 0 = transparent) */
static const uint8_t cursor_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH] = {
    {2,0,0,0,0,0,0,0,0,0,0,0},
    {2,2,0,0,0,0,0,0,0,0,0,0},
    {2,1,2,0,0,0,0,0,0,0,0,0},
    {2,1,1,2,0,0,0,0,0,0,0,0},
    {2,1,1,1,2,0,0,0,0,0,0,0},
    {2,1,1,1,1,2,0,0,0,0,0,0},
    {2,1,1,1,1,1,2,0,0,0,0,0},
    {2,1,1,1,1,1,1,2,0,0,0,0},
    {2,1,1,1,1,1,1,1,2,0,0,0},
    {2,1,1,1,1,1,1,1,1,2,0,0},
    {2,1,1,1,1,1,1,1,1,1,2,0},
    {2,1,1,1,1,1,1,2,2,2,2,2},
    {2,1,1,1,2,1,1,2,0,0,0,0},
    {2,1,1,2,0,2,1,1,2,0,0,0},
    {2,1,2,0,0,2,1,1,2,0,0,0},
    {2,2,0,0,0,0,2,1,1,2,0,0},
    {2,0,0,0,0,0,2,1,1,2,0,0},
    {0,0,0,0,0,0,0,2,1,2,0,0},
    {0,0,0,0,0,0,0,0,2,0,0,0},
};

/* Desktop state */
static int initialized = 0;
static terminal_t* main_terminal = 0;

/* Previous mouse state for click detection */
static int prev_mouse_buttons = 0;

/* Window dragging state */
static window_t* dragging_window = 0;
static int drag_offset_x = 0;
static int drag_offset_y = 0;

/* Window resizing state */
#define RESIZE_BORDER 6
#define MIN_WINDOW_WIDTH 100
#define MIN_WINDOW_HEIGHT 80

/* Resize edge flags */
#define RESIZE_NONE   0
#define RESIZE_LEFT   1
#define RESIZE_RIGHT  2
#define RESIZE_TOP    4
#define RESIZE_BOTTOM 8

static window_t* resizing_window = 0;
static int resize_edge = RESIZE_NONE;
static int resize_start_mx = 0;
static int resize_start_my = 0;
static int resize_start_x = 0;
static int resize_start_y = 0;
static int resize_start_w = 0;
static int resize_start_h = 0;

/*
 * Draw the mouse cursor at the given position
 */
static void draw_cursor(int x, int y) {
    for (int row = 0; row < CURSOR_HEIGHT; row++) {
        for (int col = 0; col < CURSOR_WIDTH; col++) {
            uint8_t pixel = cursor_bitmap[row][col];
            if (pixel == 1) {
                draw_pixel(x + col, y + row, COLOR_WHITE);
            } else if (pixel == 2) {
                draw_pixel(x + col, y + row, COLOR_BLACK);
            }
            /* pixel == 0 is transparent, skip */
        }
    }
}

/*
 * Initialize the desktop environment
 */
void desktop_init(void) {
    if (initialized) return;

    /* Initialize mouse */
    mouse_init();

    /* Initialize window manager */
    wm_init();

    /* Initialize taskbar */
    taskbar_init();

    /* Create initial terminal window */
    main_terminal = terminal_create(100, 80);

    initialized = 1;
}

/*
 * Draw the entire desktop
 */
void desktop_draw(void) {
    /* Get screen dimensions */
    int screen_h = graphics_get_height();

    /* Clear screen to desktop background color */
    /* Only clear the area above the taskbar */
    draw_filled_rect(0, 0, graphics_get_width(), screen_h - TASKBAR_HEIGHT, DESKTOP_BG_COLOR);

    /* Draw all windows */
    wm_draw_all();

    /* Draw taskbar */
    taskbar_draw();

    /* Draw mouse cursor on top of everything */
    int mx = mouse_get_x();
    int my = mouse_get_y();
    draw_cursor(mx, my);

    /* Swap buffers to display the frame */
    graphics_swap_buffers();
}

/*
 * Check if point is in window titlebar (for dragging)
 */
static int point_in_titlebar(window_t* win, int x, int y) {
    if (!win) return 0;
    /* Titlebar is at top of window, below border */
    return (x >= win->x + WINDOW_BORDER &&
            x < win->x + win->width - WINDOW_BORDER - 20 &&  /* Leave room for close button */
            y >= win->y + WINDOW_BORDER &&
            y < win->y + TITLEBAR_HEIGHT);
}

/*
 * Check which resize edge(s) the mouse is on
 * Returns a combination of RESIZE_* flags, or RESIZE_NONE
 */
static int get_resize_edge(window_t* win, int x, int y) {
    if (!win) return RESIZE_NONE;

    int edge = RESIZE_NONE;

    /* Check if point is within window bounds */
    if (x < win->x || x >= win->x + win->width ||
        y < win->y || y >= win->y + win->height) {
        return RESIZE_NONE;
    }

    /* Check left edge */
    if (x < win->x + RESIZE_BORDER) {
        edge |= RESIZE_LEFT;
    }
    /* Check right edge */
    if (x >= win->x + win->width - RESIZE_BORDER) {
        edge |= RESIZE_RIGHT;
    }
    /* Check top edge (but not titlebar area for dragging) */
    if (y < win->y + RESIZE_BORDER) {
        edge |= RESIZE_TOP;
    }
    /* Check bottom edge */
    if (y >= win->y + win->height - RESIZE_BORDER) {
        edge |= RESIZE_BOTTOM;
    }

    return edge;
}

/*
 * Check if point is in any window
 */
static window_t* find_window_at(int x, int y) {
    /* Check windows from front to back using wm_get_focused first */
    window_t* focused = wm_get_focused();
    if (focused && x >= focused->x && x < focused->x + focused->width &&
        y >= focused->y && y < focused->y + focused->height) {
        return focused;
    }
    /* For simplicity, just return focused window or NULL */
    /* Full implementation would iterate through all windows */
    return 0;
}

/*
 * Main desktop loop
 * This function never returns - it continuously:
 * 1. Draws the desktop
 * 2. Handles keyboard input
 * 3. Handles mouse input
 */
void desktop_run(void) {
    if (!initialized) {
        desktop_init();
    }

    while (1) {
        /* Draw the desktop */
        desktop_draw();

        /* Handle keyboard input */
        char key = keyboard_getchar_nonblocking();
        if (key != 0) {
            /* Forward to focused window */
            wm_handle_key(key);
        }

        /* Handle mouse input */
        int mx = mouse_get_x();
        int my = mouse_get_y();
        int buttons = 0;
        if (mouse_left_pressed()) buttons |= MOUSE_LEFT_BUTTON;
        if (mouse_right_pressed()) buttons |= MOUSE_RIGHT_BUTTON;

        int left_pressed = buttons & MOUSE_LEFT_BUTTON;
        int was_left_pressed = prev_mouse_buttons & MOUSE_LEFT_BUTTON;

        /* Handle resizing */
        if (resizing_window && left_pressed) {
            /* Continue resizing */
            int dx = mx - resize_start_mx;
            int dy = my - resize_start_my;

            int new_x = resize_start_x;
            int new_y = resize_start_y;
            int new_w = resize_start_w;
            int new_h = resize_start_h;

            if (resize_edge & RESIZE_LEFT) {
                new_x = resize_start_x + dx;
                new_w = resize_start_w - dx;
            }
            if (resize_edge & RESIZE_RIGHT) {
                new_w = resize_start_w + dx;
            }
            if (resize_edge & RESIZE_TOP) {
                new_y = resize_start_y + dy;
                new_h = resize_start_h - dy;
            }
            if (resize_edge & RESIZE_BOTTOM) {
                new_h = resize_start_h + dy;
            }

            /* Enforce minimum size */
            if (new_w < MIN_WINDOW_WIDTH) {
                if (resize_edge & RESIZE_LEFT) {
                    new_x = resize_start_x + resize_start_w - MIN_WINDOW_WIDTH;
                }
                new_w = MIN_WINDOW_WIDTH;
            }
            if (new_h < MIN_WINDOW_HEIGHT) {
                if (resize_edge & RESIZE_TOP) {
                    new_y = resize_start_y + resize_start_h - MIN_WINDOW_HEIGHT;
                }
                new_h = MIN_WINDOW_HEIGHT;
            }

            /* Keep on screen */
            if (new_x < 0) new_x = 0;
            if (new_y < 0) new_y = 0;

            resizing_window->x = new_x;
            resizing_window->y = new_y;
            resizing_window->width = new_w;
            resizing_window->height = new_h;
        } else if (resizing_window && !left_pressed) {
            /* Stop resizing */
            resizing_window = 0;
            resize_edge = RESIZE_NONE;
        }
        /* Handle dragging */
        else if (dragging_window && left_pressed) {
            /* Continue dragging - update window position */
            dragging_window->x = mx - drag_offset_x;
            dragging_window->y = my - drag_offset_y;

            /* Keep window on screen */
            if (dragging_window->x < 0) dragging_window->x = 0;
            if (dragging_window->y < 0) dragging_window->y = 0;
            int max_x = graphics_get_width() - dragging_window->width;
            int max_y = graphics_get_height() - TASKBAR_HEIGHT - dragging_window->height;
            if (dragging_window->x > max_x) dragging_window->x = max_x;
            if (dragging_window->y > max_y) dragging_window->y = max_y;
        } else if (dragging_window && !left_pressed) {
            /* Stop dragging */
            dragging_window = 0;
        } else if (left_pressed && !was_left_pressed) {
            /* Just clicked - check what was clicked */
            int taskbar_y = graphics_get_height() - TASKBAR_HEIGHT;

            if (my >= taskbar_y) {
                /* Click in taskbar */
                taskbar_handle_click(mx, my);
            } else {
                /* Click in desktop/window area */
                window_t* focused = wm_get_focused();

                /* Check for resize edge first */
                int edge = get_resize_edge(focused, mx, my);
                if (focused && edge != RESIZE_NONE) {
                    /* Start resizing */
                    resizing_window = focused;
                    resize_edge = edge;
                    resize_start_mx = mx;
                    resize_start_my = my;
                    resize_start_x = focused->x;
                    resize_start_y = focused->y;
                    resize_start_w = focused->width;
                    resize_start_h = focused->height;
                }
                /* Check if clicking on focused window's titlebar to start drag */
                else if (focused && point_in_titlebar(focused, mx, my)) {
                    dragging_window = focused;
                    drag_offset_x = mx - focused->x;
                    drag_offset_y = my - focused->y;
                } else {
                    /* Let window manager handle other clicks */
                    wm_handle_mouse(mx, my, buttons);

                    /* After handling, check if we should start dragging new focused window */
                    window_t* new_focused = wm_get_focused();
                    if (new_focused && new_focused != focused) {
                        /* Check for resize on newly focused window */
                        int new_edge = get_resize_edge(new_focused, mx, my);
                        if (new_edge != RESIZE_NONE) {
                            resizing_window = new_focused;
                            resize_edge = new_edge;
                            resize_start_mx = mx;
                            resize_start_my = my;
                            resize_start_x = new_focused->x;
                            resize_start_y = new_focused->y;
                            resize_start_w = new_focused->width;
                            resize_start_h = new_focused->height;
                        } else if (point_in_titlebar(new_focused, mx, my)) {
                            dragging_window = new_focused;
                            drag_offset_x = mx - new_focused->x;
                            drag_offset_y = my - new_focused->y;
                        }
                    }
                }
            }
        }

        prev_mouse_buttons = buttons;

        /* Small delay to avoid using 100% CPU */
        /* In a real OS, this would be replaced with proper scheduling */
        for (volatile int i = 0; i < 10000; i++);
    }
}
