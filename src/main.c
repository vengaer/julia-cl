#include "cmpxchg.h"
#include "display.h"
#include "graphics.h"
#include "julia.h"
#include "particle.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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


    if(!julia_init(MAX_ITERS)) {
        return 1;
    }

    if(!gl_create_window(width, height)) {
        julia_cleanup();
        return 1;
    }

    if(!gl_init()) {
        gl_terminate();
        julia_cleanup();
		return 1;
	}

    if(!CMPXCHG_LOCK_FREE) {
        if(!CMPXCHG_INIT(void)) {
            fputs("Failed to initialize lock-based cmpxchg\n", stderr);
            gl_terminate();
            julia_cleanup();
            return 1;
        }
    }


    if(!particle_spawn()) {
        fputs("Failed to spawn particle thread\n", stderr);
        gl_terminate();
        julia_cleanup();
        if(!CMPXCHG_LOCK_FREE) {
            CMPXCHG_CLEANUP(void);
        }
        return 1;
    }

    unsigned char *texdata = malloc(width * height * 3 * sizeof(unsigned char));
    struct complexptf ppos;

    while(!interrupted && !gl_window_should_close()) {
        ppos = particle_position();

        julia_update_constant(&ppos);
        julia_run_kernel(texdata);

        gl_clear();
        gl_update_texture(texdata);

        gl_delta_tick();
        printf("fps: %u       \r", gl_fps());

        gl_render();
        gl_update();
    }

    gl_terminate();
    free(texdata);
    julia_cleanup();

    if(!CMPXCHG_LOCK_FREE) {
        CMPXCHG_CLEANUP(void);
    }

    if(!particle_join()) {
        fputs("Failed to join particle thread\n", stderr);
    }

    return 0;
}
