#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

// Mouse state
typedef struct {
    int x;
    int y;
    uint8_t buttons;  // bit 0 = left, bit 1 = right, bit 2 = middle
} mouse_state_t;

// Button masks
#define MOUSE_LEFT_BUTTON   0x01
#define MOUSE_RIGHT_BUTTON  0x02
#define MOUSE_MIDDLE_BUTTON 0x04

void mouse_init(void);
void mouse_handler(void);
mouse_state_t mouse_get_state(void);
int mouse_get_x(void);
int mouse_get_y(void);
int mouse_left_pressed(void);
int mouse_right_pressed(void);

#endif
