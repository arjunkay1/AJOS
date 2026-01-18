#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include "graphics.h"

#define FONT_WIDTH 8
#define FONT_HEIGHT 16

void font_draw_char(int x, int y, char c, color_t fg, color_t bg);
void font_draw_string(int x, int y, const char* str, color_t fg, color_t bg);
int font_get_width(void);
int font_get_height(void);

#endif
