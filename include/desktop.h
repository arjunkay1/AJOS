#ifndef DESKTOP_H
#define DESKTOP_H

#include "graphics.h"

/* Desktop background color (teal/cyan like Windows 95) */
#define DESKTOP_BG_COLOR RGB(0, 128, 128)  /* #008080 */

/* Desktop functions */
void desktop_init(void);
void desktop_run(void);   /* Main GUI loop - never returns */
void desktop_draw(void);

#endif
