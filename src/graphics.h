#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>

bool gl_create_window(uint32_t width, uint32_t height);
void gl_terminate(void);
bool gl_window_should_close(void);
void gl_clear(void);
void gl_update(void);

bool gl_init();
void gl_update_texture(unsigned char *texdata);
void gl_render(void);

void gl_framebuffer_size(uint32_t *width, uint32_t *height);
void gl_delta_tick(void);
float gl_delta_time(void);
unsigned gl_fps(void);

#endif
