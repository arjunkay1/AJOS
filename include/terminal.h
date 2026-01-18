#ifndef TERMINAL_H
#define TERMINAL_H

#include "window.h"

#define TERM_COLS 80
#define TERM_ROWS 24

typedef struct {
    window_t* window;
    char buffer[TERM_ROWS][TERM_COLS + 1];
    int cursor_row;
    int cursor_col;
    color_t fg_color;
    color_t bg_color;
    char input_line[256];
    int input_pos;
} terminal_t;

terminal_t* terminal_create(int x, int y);
void terminal_destroy(terminal_t* term);
void terminal_putchar(terminal_t* term, char c);
void terminal_print(terminal_t* term, const char* str);
void terminal_clear(terminal_t* term);
void terminal_draw(terminal_t* term);
void terminal_handle_key(terminal_t* term, char key);
void terminal_scroll(terminal_t* term);

#endif
