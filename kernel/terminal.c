/*
 * AJOS Graphical Terminal
 * A terminal window for the desktop environment
 */

#include "terminal.h"
#include "window.h"
#include "graphics.h"
#include "font.h"
#include "string.h"

/* Terminal colors */
#define TERM_BG_COLOR   COLOR_BLACK
#define TERM_FG_COLOR   RGB(192, 192, 192)  /* Light gray text */
#define TERM_PROMPT_COLOR RGB(0, 255, 0)    /* Green prompt */

/* Global terminal instance (for callback access) */
static terminal_t* g_terminal = 0;

/* Forward declarations for command processing */
static void terminal_process_command(terminal_t* term);
static void terminal_show_prompt(terminal_t* term);

/*
 * Draw callback for the terminal window
 */
static void terminal_draw_callback(window_t* win) {
    if (!g_terminal || g_terminal->window != win) {
        return;
    }
    terminal_draw(g_terminal);
}

/*
 * Key callback for the terminal window
 */
static void terminal_key_callback(window_t* win, char key) {
    if (!g_terminal || g_terminal->window != win) {
        return;
    }
    terminal_handle_key(g_terminal, key);
}

/*
 * Create a new terminal window
 */
terminal_t* terminal_create(int x, int y) {
    /* Allocate terminal structure (static for now - no malloc) */
    static terminal_t term_storage;
    terminal_t* term = &term_storage;

    /* Calculate window size based on terminal dimensions */
    /* Add padding for borders and some margin */
    int char_width = font_get_width();
    int char_height = font_get_height();
    int content_width = TERM_COLS * char_width + 8;   /* 4px padding each side */
    int content_height = TERM_ROWS * char_height + 8;

    /* Total window size includes titlebar and borders */
    int win_width = content_width + 2 * WINDOW_BORDER + 4;
    int win_height = content_height + TITLEBAR_HEIGHT + 2 * WINDOW_BORDER + 4;

    /* Create the window */
    term->window = wm_create_window(x, y, win_width, win_height, "Terminal");
    if (!term->window) {
        return 0;
    }

    /* Set window background to terminal background */
    term->window->bg_color = TERM_BG_COLOR;

    /* Set callbacks */
    term->window->draw_content = terminal_draw_callback;
    term->window->on_key = terminal_key_callback;

    /* Initialize terminal state */
    term->cursor_row = 0;
    term->cursor_col = 0;
    term->fg_color = TERM_FG_COLOR;
    term->bg_color = TERM_BG_COLOR;
    term->input_pos = 0;

    /* Clear buffer */
    for (int row = 0; row < TERM_ROWS; row++) {
        for (int col = 0; col <= TERM_COLS; col++) {
            term->buffer[row][col] = '\0';
        }
    }
    memset(term->input_line, 0, sizeof(term->input_line));

    /* Store global reference */
    g_terminal = term;

    /* Show welcome message and prompt */
    terminal_print(term, "AJOS Terminal v0.1\n");
    terminal_print(term, "Type 'aj help' for commands.\n\n");
    terminal_show_prompt(term);

    return term;
}

/*
 * Destroy a terminal
 */
void terminal_destroy(terminal_t* term) {
    if (!term) return;

    if (term->window) {
        wm_destroy_window(term->window);
        term->window = 0;
    }

    if (g_terminal == term) {
        g_terminal = 0;
    }
}

/*
 * Scroll the terminal buffer up by one line
 */
void terminal_scroll(terminal_t* term) {
    if (!term) return;

    /* Move all lines up by one */
    for (int row = 0; row < TERM_ROWS - 1; row++) {
        for (int col = 0; col <= TERM_COLS; col++) {
            term->buffer[row][col] = term->buffer[row + 1][col];
        }
    }

    /* Clear the last line */
    for (int col = 0; col <= TERM_COLS; col++) {
        term->buffer[TERM_ROWS - 1][col] = '\0';
    }
}

/*
 * Put a character to the terminal buffer
 */
void terminal_putchar(terminal_t* term, char c) {
    if (!term) return;

    if (c == '\n') {
        /* Newline - move to start of next line */
        term->buffer[term->cursor_row][term->cursor_col] = '\0';
        term->cursor_col = 0;
        term->cursor_row++;

        if (term->cursor_row >= TERM_ROWS) {
            terminal_scroll(term);
            term->cursor_row = TERM_ROWS - 1;
        }
    } else if (c == '\r') {
        /* Carriage return - move to start of line */
        term->cursor_col = 0;
    } else if (c == '\b') {
        /* Backspace - move back one character */
        if (term->cursor_col > 0) {
            term->cursor_col--;
            term->buffer[term->cursor_row][term->cursor_col] = ' ';
        }
    } else if (c == '\t') {
        /* Tab - move to next 4-character boundary */
        int next_tab = ((term->cursor_col / 4) + 1) * 4;
        while (term->cursor_col < next_tab && term->cursor_col < TERM_COLS) {
            term->buffer[term->cursor_row][term->cursor_col] = ' ';
            term->cursor_col++;
        }
    } else if (c >= 32 && c < 127) {
        /* Printable character */
        if (term->cursor_col < TERM_COLS) {
            term->buffer[term->cursor_row][term->cursor_col] = c;
            term->cursor_col++;

            /* Wrap to next line if at end */
            if (term->cursor_col >= TERM_COLS) {
                term->cursor_col = 0;
                term->cursor_row++;
                if (term->cursor_row >= TERM_ROWS) {
                    terminal_scroll(term);
                    term->cursor_row = TERM_ROWS - 1;
                }
            }
        }
    }
}

/*
 * Print a string to the terminal
 */
void terminal_print(terminal_t* term, const char* str) {
    if (!term || !str) return;

    while (*str) {
        terminal_putchar(term, *str);
        str++;
    }
}

/*
 * Clear the terminal
 */
void terminal_clear(terminal_t* term) {
    if (!term) return;

    /* Clear all buffer contents */
    for (int row = 0; row < TERM_ROWS; row++) {
        for (int col = 0; col <= TERM_COLS; col++) {
            term->buffer[row][col] = '\0';
        }
    }

    /* Reset cursor */
    term->cursor_row = 0;
    term->cursor_col = 0;
}

/*
 * Draw the terminal contents
 */
void terminal_draw(terminal_t* term) {
    if (!term || !term->window) return;

    /* Get content area coordinates */
    int base_x = wm_content_x(term->window) + 4;  /* Small padding */
    int base_y = wm_content_y(term->window) + 4;

    int char_width = font_get_width();
    int char_height = font_get_height();

    /* Draw each character in the buffer */
    for (int row = 0; row < TERM_ROWS; row++) {
        for (int col = 0; col < TERM_COLS; col++) {
            char c = term->buffer[row][col];
            if (c == '\0') {
                /* Draw space for empty cells */
                c = ' ';
            }

            int x = base_x + col * char_width;
            int y = base_y + row * char_height;

            font_draw_char(x, y, c, term->fg_color, term->bg_color);
        }
    }

    /* Draw cursor (blinking block) */
    int cursor_x = base_x + term->cursor_col * char_width;
    int cursor_y = base_y + term->cursor_row * char_height;
    draw_filled_rect(cursor_x, cursor_y, char_width, char_height, term->fg_color);
}

/*
 * Show the command prompt
 */
static void terminal_show_prompt(terminal_t* term) {
    terminal_print(term, "AJOS> ");
}

/*
 * Process a command entered in the terminal
 */
static void terminal_process_command(terminal_t* term) {
    if (!term) return;

    /* Null-terminate the input */
    term->input_line[term->input_pos] = '\0';

    /* Skip leading whitespace */
    char* cmd = term->input_line;
    while (*cmd == ' ' || *cmd == '\t') cmd++;

    /* Check if empty */
    if (*cmd == '\0') {
        terminal_show_prompt(term);
        return;
    }

    /* Parse command - check for "aj" prefix */
    if (strncmp(cmd, "aj ", 3) == 0 || strcmp(cmd, "aj") == 0) {
        char* subcmd = cmd + 2;
        while (*subcmd == ' ') subcmd++;

        if (*subcmd == '\0') {
            terminal_print(term, "Usage: aj <command>\n");
            terminal_print(term, "Type 'aj help' for a list of commands.\n");
        } else if (strcmp(subcmd, "help") == 0) {
            terminal_print(term, "Available commands:\n");
            terminal_print(term, "  aj help    - Show this help\n");
            terminal_print(term, "  aj clear   - Clear terminal\n");
            terminal_print(term, "  aj version - Show version\n");
            terminal_print(term, "  aj echo <text> - Print text\n");
            terminal_print(term, "  aj reboot  - Reboot system\n");
            terminal_print(term, "  aj halt    - Halt CPU\n");
        } else if (strcmp(subcmd, "clear") == 0) {
            terminal_clear(term);
        } else if (strcmp(subcmd, "version") == 0) {
            terminal_print(term, "AJOS v1.0.0\n");
        } else if (strncmp(subcmd, "echo ", 5) == 0) {
            terminal_print(term, subcmd + 5);
            terminal_print(term, "\n");
        } else if (strcmp(subcmd, "echo") == 0) {
            terminal_print(term, "\n");
        } else if (strcmp(subcmd, "reboot") == 0) {
            terminal_print(term, "Rebooting...\n");
            /* Send reset command to keyboard controller */
            __asm__ volatile (
                "1: inb $0x64, %%al\n"
                "   testb $0x02, %%al\n"
                "   jnz 1b\n"
                "   movb $0xFE, %%al\n"
                "   outb %%al, $0x64\n"
                : : : "al"
            );
            __asm__ volatile ("cli; hlt");
        } else if (strcmp(subcmd, "halt") == 0) {
            terminal_print(term, "System halted.\n");
            __asm__ volatile ("cli; hlt");
        } else {
            terminal_print(term, "Unknown command: aj ");
            terminal_print(term, subcmd);
            terminal_print(term, "\nType 'aj help' for commands.\n");
        }
    } else {
        terminal_print(term, "Unknown command: ");
        terminal_print(term, cmd);
        terminal_print(term, "\nCommands use 'aj' prefix. Type 'aj help' for help.\n");
    }

    terminal_show_prompt(term);
}

/*
 * Handle keyboard input
 */
void terminal_handle_key(terminal_t* term, char key) {
    if (!term) return;

    if (key == '\n') {
        /* Enter pressed - process command */
        terminal_putchar(term, '\n');
        terminal_process_command(term);

        /* Clear input buffer */
        memset(term->input_line, 0, sizeof(term->input_line));
        term->input_pos = 0;
    } else if (key == '\b') {
        /* Backspace pressed */
        if (term->input_pos > 0) {
            term->input_pos--;
            term->input_line[term->input_pos] = '\0';
            terminal_putchar(term, '\b');
        }
    } else if (key >= 32 && key < 127) {
        /* Printable character */
        if (term->input_pos < 255) {
            term->input_line[term->input_pos] = key;
            term->input_pos++;
            terminal_putchar(term, key);
        }
    }
}
