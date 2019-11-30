#include "display.h"
#include "graphics.h"
#include "particle.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <signal.h>

bool volatile interrupted = false;

void signal_handler(int signal) {
    if(signal == SIGINT || signal == SIGKILL) {
        interrupted = true;
    }
}

int main(void) {
    unsigned width, height;

    if(!display_get_resolution(&width, &height)) {
        return 1;
    }
    printf("Window resolution: %ux%u\n", width, height);

    unsigned char *texdata = malloc(width * height * 3 * sizeof(unsigned char));

    if(!gl_create_window(width, height)) {
        free(texdata);
        return 1;
    }

	for(unsigned i = 0; i < width * height * 3; i += 3) {
		texdata[i] = 255;
		texdata[i + 1] = 0;
		texdata[i + 2] = 0;
	}
    if(!gl_init(texdata)) {
		free(texdata);
		return 1;
	}

    if(!particle_spawn()) {
        fputs("Failed to spawn particle thread", stderr);
        free(texdata);
        return 1;
    }

    struct complexptf p;
    while(!interrupted && !gl_window_should_close()) {
        gl_clear();
        //p = particle_position();
        //complexptf_print(&p);
        gl_render();
        gl_update_texture(texdata);
        gl_update();
    }

    gl_terminate();
    free(texdata);

    if(!particle_join()) {
        fputs("Failed to join particle thread", stderr);
        return 1;
    }

    return 0;
}
