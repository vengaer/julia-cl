#include "display.h"

#include <stdio.h>

#include <X11/Xlib.h>

bool display_get_resolution(unsigned *width, unsigned *height) {
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

