#ifndef TASKBAR_H
#define TASKBAR_H

#include "graphics.h"

#define TASKBAR_HEIGHT 32

void taskbar_init(void);
void taskbar_draw(void);
void taskbar_handle_click(int x, int y);

#endif
