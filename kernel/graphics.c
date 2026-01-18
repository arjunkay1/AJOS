/*
 * AJOS Graphics Driver
 * VESA VBE framebuffer graphics implementation
 */

#include "graphics.h"

/* Multiboot info flag for framebuffer info valid (bit 12) */
#define MULTIBOOT_FLAG_FRAMEBUFFER (1 << 12)

/* Framebuffer type values */
#define FRAMEBUFFER_TYPE_INDEXED  0
#define FRAMEBUFFER_TYPE_RGB      1
#define FRAMEBUFFER_TYPE_TEXT     2

/* Global graphics info structure - used by draw.c */
graphics_info_t g_graphics = {
    .framebuffer = 0,
    .width = 0,
    .height = 0,
    .pitch = 0,
    .bpp = 0,
    .initialized = 0
};

/* Double buffer for flicker-free rendering */
/* Allocate 800x600x4 bytes = 1920000 bytes (~1.8MB) */
static uint32_t back_buffer[800 * 600];
static uint32_t* front_buffer = 0;

/*
 * Initialize graphics from multiboot info
 * Parses the multiboot structure to extract framebuffer information
 *
 * Multiboot info structure offsets:
 *   0: flags (uint32_t)
 *   88: framebuffer_addr (uint64_t)
 *   96: framebuffer_pitch (uint32_t)
 *   100: framebuffer_width (uint32_t)
 *   104: framebuffer_height (uint32_t)
 *   108: framebuffer_bpp (uint8_t)
 *   109: framebuffer_type (uint8_t)
 */
void graphics_init(void* multiboot_info) {
    if (multiboot_info == 0) {
        return;
    }

    /* Cast to byte pointer for offset calculations */
    uint8_t* mb_info = (uint8_t*)multiboot_info;

    /* Read flags from offset 0 */
    uint32_t flags = *((uint32_t*)(mb_info + 0));

    /* Check if framebuffer info is available (bit 12) */
    if (!(flags & MULTIBOOT_FLAG_FRAMEBUFFER)) {
        return;
    }

    /* Read framebuffer address from offset 88 (64-bit) */
    /* For 32-bit system, we only use the lower 32 bits */
    uint32_t fb_addr_low = *((uint32_t*)(mb_info + 88));
    /* uint32_t fb_addr_high = *((uint32_t*)(mb_info + 92)); */

    /* Read framebuffer pitch from offset 96 */
    uint32_t fb_pitch = *((uint32_t*)(mb_info + 96));

    /* Read framebuffer width from offset 100 */
    uint32_t fb_width = *((uint32_t*)(mb_info + 100));

    /* Read framebuffer height from offset 104 */
    uint32_t fb_height = *((uint32_t*)(mb_info + 104));

    /* Read framebuffer bpp from offset 108 */
    uint8_t fb_bpp = *((uint8_t*)(mb_info + 108));

    /* Read framebuffer type from offset 109 */
    uint8_t fb_type = *((uint8_t*)(mb_info + 109));

    /* We only support RGB direct color mode (type 1) */
    if (fb_type != FRAMEBUFFER_TYPE_RGB) {
        return;
    }

    /* Store framebuffer info */
    front_buffer = (uint32_t*)(uintptr_t)fb_addr_low;
    g_graphics.framebuffer = back_buffer;  /* Draw to back buffer */
    g_graphics.width = fb_width;
    g_graphics.height = fb_height;
    g_graphics.pitch = fb_width * 4;  /* Back buffer is tightly packed */
    g_graphics.bpp = fb_bpp;
    g_graphics.initialized = 1;
}

/*
 * Check if graphics mode is available
 * Returns 1 if framebuffer is initialized, 0 otherwise
 */
int graphics_is_available(void) {
    return g_graphics.initialized;
}

/*
 * Get framebuffer width in pixels
 */
uint32_t graphics_get_width(void) {
    return g_graphics.width;
}

/*
 * Get framebuffer height in pixels
 */
uint32_t graphics_get_height(void) {
    return g_graphics.height;
}

/*
 * Swap buffers - copy back buffer to front buffer
 * This is called once per frame after all drawing is complete
 */
void graphics_swap_buffers(void) {
    if (!g_graphics.initialized || !front_buffer) {
        return;
    }

    /* Copy back buffer to front buffer */
    uint32_t* src = back_buffer;
    uint32_t* dst = front_buffer;
    uint32_t pixels = g_graphics.width * g_graphics.height;

    for (uint32_t i = 0; i < pixels; i++) {
        dst[i] = src[i];
    }
}
