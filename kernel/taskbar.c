/*
 * AJOS Taskbar
 * Desktop taskbar with start button, window buttons, and clock
 */

#include "taskbar.h"
#include "graphics.h"
#include "font.h"
#include "window.h"
#include "terminal.h"
#include "rtc.h"

/* Taskbar colors */
#define TASKBAR_BG_COLOR      RGB(64, 64, 64)    /* Dark gray (#404040) */
#define TASKBAR_BORDER_LIGHT  RGB(128, 128, 128)
#define TASKBAR_BORDER_DARK   RGB(32, 32, 32)
#define START_BTN_COLOR       RGB(0, 128, 0)     /* Green */
#define START_BTN_TEXT        COLOR_WHITE
#define CLOCK_TEXT_COLOR      COLOR_WHITE
#define WINDOW_BTN_COLOR      RGB(96, 96, 96)
#define WINDOW_BTN_ACTIVE     RGB(128, 128, 160)
#define WINDOW_BTN_TEXT       COLOR_WHITE

/* Button dimensions */
#define START_BTN_WIDTH  60
#define START_BTN_HEIGHT 24
#define START_BTN_MARGIN 4

#define WINDOW_BTN_WIDTH  120
#define WINDOW_BTN_HEIGHT 24
#define WINDOW_BTN_MARGIN 4

#define CLOCK_WIDTH 96

/* Taskbar state */
static int taskbar_y = 0;
static int screen_w = 0;

/* External terminal reference for creating new terminals */
extern terminal_t* terminal_create(int x, int y);

/*
 * Initialize the taskbar
 */
void taskbar_init(void) {
    screen_w = graphics_get_width();
    taskbar_y = graphics_get_height() - TASKBAR_HEIGHT;
}

/*
 * Draw a 3D button
 */
static void draw_button(int x, int y, int w, int h, color_t bg_color, int pressed) {
    /* Fill background */
    draw_filled_rect(x, y, w, h, bg_color);

    if (pressed) {
        /* Pressed: dark top-left, light bottom-right */
        draw_hline(x, y, w, TASKBAR_BORDER_DARK);
        draw_vline(x, y, h, TASKBAR_BORDER_DARK);
        draw_hline(x, y + h - 1, w, TASKBAR_BORDER_LIGHT);
        draw_vline(x + w - 1, y, h, TASKBAR_BORDER_LIGHT);
    } else {
        /* Normal: light top-left, dark bottom-right */
        draw_hline(x, y, w, TASKBAR_BORDER_LIGHT);
        draw_vline(x, y, h, TASKBAR_BORDER_LIGHT);
        draw_hline(x, y + h - 1, w, TASKBAR_BORDER_DARK);
        draw_vline(x + w - 1, y, h, TASKBAR_BORDER_DARK);
    }
}

/*
 * Draw the taskbar
 */
void taskbar_draw(void) {
    if (screen_w == 0) {
        taskbar_init();
    }

    /* Draw taskbar background */
    draw_filled_rect(0, taskbar_y, screen_w, TASKBAR_HEIGHT, TASKBAR_BG_COLOR);

    /* Top border highlight */
    draw_hline(0, taskbar_y, screen_w, TASKBAR_BORDER_LIGHT);

    /* Draw Start button */
    int start_x = START_BTN_MARGIN;
    int start_y = taskbar_y + (TASKBAR_HEIGHT - START_BTN_HEIGHT) / 2;
    draw_button(start_x, start_y, START_BTN_WIDTH, START_BTN_HEIGHT, START_BTN_COLOR, 0);

    /* Draw "AJOS" text on start button */
    int text_x = start_x + (START_BTN_WIDTH - 4 * font_get_width()) / 2;
    int text_y = start_y + (START_BTN_HEIGHT - font_get_height()) / 2;
    font_draw_string(text_x, text_y, "AJOS", START_BTN_TEXT, START_BTN_COLOR);

    /* Draw clock on the right side */
    int clock_x = screen_w - CLOCK_WIDTH - START_BTN_MARGIN;
    int clock_y = taskbar_y + (TASKBAR_HEIGHT - font_get_height()) / 2;

    /* Get current time from RTC */
    rtc_time_t time;
    rtc_get_time(&time);

    /* Apply timezone offset: Eastern Time (UTC-5) */
    int hours = time.hours;
    hours = hours - 5;  /* EST offset */
    if (hours < 0) hours += 24;

    /* Convert to 12-hour format */
    int is_pm = (hours >= 12);
    int hours12 = hours % 12;
    if (hours12 == 0) hours12 = 12;

    /* Format time string (H:MM:SS AM/PM) */
    char time_str[12];
    int i = 0;

    /* Hours (no leading zero for 12-hour format) */
    if (hours12 >= 10) {
        time_str[i++] = '0' + (hours12 / 10);
    }
    time_str[i++] = '0' + (hours12 % 10);
    time_str[i++] = ':';

    /* Minutes */
    time_str[i++] = '0' + (time.minutes / 10);
    time_str[i++] = '0' + (time.minutes % 10);
    time_str[i++] = ':';

    /* Seconds */
    time_str[i++] = '0' + (time.seconds / 10);
    time_str[i++] = '0' + (time.seconds % 10);
    time_str[i++] = ' ';

    /* AM/PM */
    time_str[i++] = is_pm ? 'P' : 'A';
    time_str[i++] = 'M';
    time_str[i] = '\0';

    font_draw_string(clock_x, clock_y, time_str, CLOCK_TEXT_COLOR, TASKBAR_BG_COLOR);

    /* Draw window buttons in the middle area */
    /* Window buttons start after start button and end before clock */
    int btn_area_start = START_BTN_MARGIN + START_BTN_WIDTH + WINDOW_BTN_MARGIN;
    int btn_area_end = clock_x - WINDOW_BTN_MARGIN;
    int btn_y = taskbar_y + (TASKBAR_HEIGHT - WINDOW_BTN_HEIGHT) / 2;

    /* Iterate through windows and draw buttons */
    int btn_x = btn_area_start;
    window_t* focused = wm_get_focused();

    for (int i = 0; ; i++) {
        /* Get window by checking slot occupancy */
        /* We need to iterate through z-order, but we don't have direct access */
        /* For now, draw up to MAX_WINDOWS buttons checking each slot */
        extern window_t* wm_get_window_by_slot(int slot);  /* Forward declaration */

        /* Simple approach: iterate through potential windows */
        /* Since we don't have a direct accessor, we'll skip window buttons for now */
        /* The window manager would need to expose this functionality */
        break;  /* Skip window buttons for now */
    }

    (void)btn_area_end;
    (void)btn_x;
    (void)btn_y;
    (void)focused;
}

/*
 * Handle click on the taskbar
 */
void taskbar_handle_click(int x, int y) {
    /* Check if click is in taskbar area */
    if (y < taskbar_y || y >= taskbar_y + TASKBAR_HEIGHT) {
        return;
    }

    /* Check if start button was clicked */
    int start_x = START_BTN_MARGIN;
    int start_y = taskbar_y + (TASKBAR_HEIGHT - START_BTN_HEIGHT) / 2;

    if (x >= start_x && x < start_x + START_BTN_WIDTH &&
        y >= start_y && y < start_y + START_BTN_HEIGHT) {
        /* Start button clicked - open a new terminal */
        terminal_create(50, 50);
        return;
    }

    /* Check if a window button was clicked */
    /* (Window button handling would go here) */
}
