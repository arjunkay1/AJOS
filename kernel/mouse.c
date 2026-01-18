#include "../include/mouse.h"
#include "../include/io.h"
#include "../include/pic.h"

static mouse_state_t mouse;
static uint8_t mouse_cycle = 0;
static int8_t mouse_bytes[3];

// Default screen bounds (can be updated if graphics mode is available)
static int screen_width = 800;
static int screen_height = 600;

// Wait for mouse controller
static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        // Wait for bit 0 (output buffer full)
        while (timeout--) {
            if (inb(0x64) & 1) return;
        }
    } else {
        // Wait for bit 1 clear (input buffer empty)
        while (timeout--) {
            if (!(inb(0x64) & 2)) return;
        }
    }
}

static void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(0x64, 0xD4);  // Tell controller we're sending to mouse
    mouse_wait(1);
    outb(0x60, data);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(0x60);
}

void mouse_init(void) {
    // Initialize mouse position to center
    mouse.x = screen_width / 2;
    mouse.y = screen_height / 2;
    mouse.buttons = 0;

    // Enable auxiliary device (mouse)
    mouse_wait(1);
    outb(0x64, 0xA8);

    // Enable interrupts
    mouse_wait(1);
    outb(0x64, 0x20);  // Get compaq status
    mouse_wait(0);
    uint8_t status = inb(0x60) | 2;  // Enable IRQ12
    mouse_wait(1);
    outb(0x64, 0x60);  // Set compaq status
    mouse_wait(1);
    outb(0x60, status);

    // Use default settings
    mouse_write(0xF6);
    mouse_read();  // ACK

    // Enable mouse
    mouse_write(0xF4);
    mouse_read();  // ACK

    // Enable IRQ12
    pic_clear_mask(12);
}

void mouse_handler(void) {
    uint8_t data = inb(0x60);

    switch (mouse_cycle) {
        case 0:
            mouse_bytes[0] = data;
            if (data & 0x08) {  // Valid first byte has bit 3 set
                mouse_cycle++;
            }
            break;
        case 1:
            mouse_bytes[1] = data;
            mouse_cycle++;
            break;
        case 2:
            mouse_bytes[2] = data;
            mouse_cycle = 0;

            // Process packet
            mouse.buttons = mouse_bytes[0] & 0x07;

            // X movement (signed)
            int dx = mouse_bytes[1];
            if (mouse_bytes[0] & 0x10) dx |= 0xFFFFFF00;  // Sign extend

            // Y movement (signed, inverted)
            int dy = mouse_bytes[2];
            if (mouse_bytes[0] & 0x20) dy |= 0xFFFFFF00;  // Sign extend

            // Update position
            mouse.x += dx;
            mouse.y -= dy;  // Invert Y

            // Clamp to screen bounds
            if (mouse.x < 0) mouse.x = 0;
            if (mouse.y < 0) mouse.y = 0;
            if (mouse.x >= screen_width) mouse.x = screen_width - 1;
            if (mouse.y >= screen_height) mouse.y = screen_height - 1;
            break;
    }
}

mouse_state_t mouse_get_state(void) { return mouse; }
int mouse_get_x(void) { return mouse.x; }
int mouse_get_y(void) { return mouse.y; }
int mouse_left_pressed(void) { return mouse.buttons & MOUSE_LEFT_BUTTON; }
int mouse_right_pressed(void) { return mouse.buttons & MOUSE_RIGHT_BUTTON; }
