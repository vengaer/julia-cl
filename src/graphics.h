#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

bool g_create_window(unsigned width, unsigned height);
void g_terminate(void);
bool g_window_should_close(void);
void g_clear(void);
void g_update(void);

bool g_init(unsigned char *texdata);
void g_update_texture(unsigned char *texdata);
void g_render(void);


#endif
