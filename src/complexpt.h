#ifndef COMPLEXPT_H
#define COMPLEXPT_H

#include <stdint.h>

struct complexpti {
    int32_t re, im;
};

struct complexptf {
    float re, im;
};

float complexptf_dist(struct complexptf const *p1, struct complexptf const *p2);
float complexptf_magnitude(struct complexptf const *p);
float complexptf_dot(struct complexptf const *p1, struct complexptf const *p2);
void complexptf_normalize(struct complexptf *p);
void complexptf_print(struct complexptf const *p);

#endif
