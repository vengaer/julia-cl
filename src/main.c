#include "graphics.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <X11/Xlib.h>

static bool volatile interrupted = false;

void signal_handler(int signal) {
    if(signal == SIGINT || signal == SIGKILL) {
        interrupted = true;
    }
}

static bool get_resolution(unsigned *width, unsigned *height) {
    Display *dsp = NULL;
    Screen *scr = NULL;

    dsp = XOpenDisplay(NULL);

    if(!dsp) {
        fputs("Failed to get display\n", stderr);
        return false;
    }

    scr = DefaultScreenOfDisplay(dsp);

    if(!scr) {
        fputs("Failed to get screen\n", stderr);
        XCloseDisplay(dsp);
        return false;
    }

    *width = scr->width;
    *height = scr->height;
    XCloseDisplay(dsp);
    return true;
} 

int main(void) {
    unsigned width, height;

    if(!get_resolution(&width, &height)) {
        return 1;
    }
    printf("Window resolution: %ux%u\n", width, height);

    unsigned char *texdata = malloc(width * height * 3 * sizeof(unsigned char));

    if(!g_create_window(width, height)) {
        free(texdata);
        return 1;
    }

	for(unsigned i = 0; i < width * height * 3; i += 3) {
		texdata[i] = 255;
		texdata[i + 1] = 0;
		texdata[i + 2] = 0;
	}
    if(!g_init(texdata)) {
		free(texdata);
		return 1;
	}


    while(!interrupted && !g_window_should_close()) {
        g_clear();
        g_render();
        /* TODO: Create tex buffer */
        g_update_texture(texdata);
        g_update();
    }

    g_terminate();
    free(texdata);

    return 0;
}