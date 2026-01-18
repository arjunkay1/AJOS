#ifndef TERMINAL_H
#define TERMINAL_H

#include "window.h"

#define TERM_COLS 80
#define TERM_ROWS 24
#define HISTORY_SIZE 16
#define MAX_INPUT_LEN 256

typedef struct {
    window_t* window;
    char buffer[TERM_ROWS][TERM_COLS + 1];
    int cursor_row;
    int cursor_col;
    color_t fg_color;
    color_t bg_color;
    char input_line[MAX_INPUT_LEN];
    int input_pos;
    /* Command history */
    char history[HISTORY_SIZE][MAX_INPUT_LEN];
    int history_count;
    int history_index;
    /* Saved input line when browsing history */
    char saved_input[MAX_INPUT_LEN];
    int saved_input_pos;
    int browsing_history;
} terminal_t;

terminal_t* terminal_create(int x, int y);
void terminal_destroy(terminal_t* term);
void terminal_putchar(terminal_t* term, char c);
void terminal_print(terminal_t* term, const char* str);
void terminal_clear(terminal_t* term);
void terminal_draw(terminal_t* term);
void terminal_handle_key(terminal_t* term, unsigned char key);
void terminal_scroll(terminal_t* term);

#endif
