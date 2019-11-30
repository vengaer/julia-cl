#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

bool gl_create_window(unsigned width, unsigned height);
void gl_terminate(void);
bool gl_window_should_close(void);
void gl_clear(void);
void gl_update(void);

bool gl_init(unsigned char *texdata);
void gl_update_texture(unsigned char *texdata);
void gl_render(void);


#endif
