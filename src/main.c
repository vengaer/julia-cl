#include "display.h"
#include "graphics.h"
#include "julia.h"
#include "particle.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <signal.h>

#define MAX_ITERS 1024

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

    if(!julia_init(MAX_ITERS)) {
        free(texdata);
        return 1;
    }

    if(!gl_init(texdata)) {
		free(texdata);
        julia_cleanup();
		return 1;
	}

    if(!particle_spawn()) {
        fputs("Failed to spawn particle thread", stderr);
        free(texdata);
        julia_cleanup();
        return 1;
    }


    struct complexptf ppos;

    while(!interrupted && !gl_window_should_close()) {
        gl_clear();
        gl_render();

        ppos = particle_position();

        julia_update_constant(&ppos);
        julia_run_kernel(texdata);

        gl_delta_tick();
        printf("fps: %u     \r", gl_fps());

        gl_update_texture(texdata);
        gl_update();
    }

    gl_terminate();
    free(texdata);
    julia_cleanup();

    if(!particle_join()) {
        fputs("Failed to join particle thread", stderr);
        return 1;
    }

    return 0;
}
