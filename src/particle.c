#include "cmpxchg.h"
#include "julia.h"
#include "particle.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#define IGNORE(x) (void)(x)
#define REFLECT(component, incident, normal, angle) \
    (incident)->component - 2.f * angle * (normal)->component

#define PARTICLE_DIR_MAX 5
#define START_POS_MIN -500
#define START_POS_MAX  500
#define RES_MULT 1000.f
#define TRANSLATION_SPEED 0.000004f

static pthread_t thread;

static bool volatile particle_alive = true;

static struct timeval delta_start, delta_stop;
static float delta_time = 0.f;

static struct complexptf const origin = { .re = 0.f, .im = 0.f };

static struct {
    struct complexpti pos;
    struct complexpti dir;
} particle;

static void update_delta_time(void) {
    gettimeofday(&delta_stop, NULL);
    delta_time = ((delta_stop.tv_sec - delta_start.tv_sec) * 1000000.f + delta_stop.tv_usec - delta_start.tv_usec) / 1000.f;
    delta_start = delta_stop;
}

static struct complexptf complexpti2f(struct complexpti const *i) {
    struct complexptf f = {
        .re = ((float)i->re) / RES_MULT,
        .im = ((float)i->im) / RES_MULT
    };
    return f;
}

static inline int32_t rand32_in_range(int32_t low, int32_t high) {
    return low + (int32_t)((((double)rand()) / (double)RAND_MAX) * high);
}

static void collide(struct complexptf const *pos, struct complexptf *dir) {
    struct complexptf normal = {
        .re = -pos->re,
        .im = -pos->im
    };
    complexptf_normalize(&normal);

    float angle = complexptf_dot(dir, &normal);

    struct complexptf new_dir = {
        .re = REFLECT(re, dir, &normal, angle),
        .im = REFLECT(im, dir, &normal, angle)
    };

    *dir = new_dir;
}

static void move(void) {
    struct complexptf pos = complexpti2f(&particle.pos);
    struct complexptf dir = complexpti2f(&particle.dir);
    complexptf_normalize(&dir);

    pos.re += dir.re * delta_time * TRANSLATION_SPEED;
    pos.im += dir.im * delta_time * TRANSLATION_SPEED;

    float dist = complexptf_dist(&pos, &origin);
    if(dist > JULIA_RADIUS) {
        puts("Collide");
        collide(&pos, &dir);
    }
    else {
        atomic_write32(&particle.pos.re, (int32_t)(pos.re * RES_MULT));
        atomic_write32(&particle.pos.im, (int32_t)(pos.im * RES_MULT));
    }

    atomic_write32(&particle.dir.re, (int32_t)(dir.re * RES_MULT));
    atomic_write32(&particle.dir.im, (int32_t)(dir.im * RES_MULT));
}

static void *handle_particle(void *args) {
    IGNORE(args);

    while(particle_alive) {
        update_delta_time();
        move();
        usleep(1000000);
    }
    return NULL;
}

bool particle_spawn(void) {
    srand(time(NULL));
    particle.dir.re = rand32_in_range(0, PARTICLE_DIR_MAX);
    particle.dir.im = rand32_in_range(0, PARTICLE_DIR_MAX);

    particle.pos.re = rand32_in_range(START_POS_MIN, START_POS_MAX);
    particle.pos.im = rand32_in_range(START_POS_MIN, START_POS_MAX);

    gettimeofday(&delta_start, NULL);
    gettimeofday(&delta_stop, NULL);
    
    return pthread_create(&thread, NULL, handle_particle, NULL) == 0;
}

bool particle_join(void) {
    particle_alive = false;
    return pthread_join(thread, NULL);
}

struct complexptf particle_position(void) {
    return complexpti2f(&particle.pos);
}
