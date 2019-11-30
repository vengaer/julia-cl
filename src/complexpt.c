#include "complexpt.h"

#include <math.h>
#include <stdio.h>

float complexptf_dist(struct complexptf const *p1, struct complexptf const *p2) {
    float d_re = p1->re - p2->re;
    float d_im = p1->im - p2->im;
    return sqrtf(d_re * d_re + d_im * d_im);
}

float complexptf_magnitude(struct complexptf const *p) {
    return sqrtf(p->re * p->re + p->im * p->im);
}

float complexptf_dot(struct complexptf const *p1, struct complexptf const *p2) {
    return p1->re * p2->re + p1->im * p2->im;
}

void complexptf_normalize(struct complexptf *p) {
    float magnitude = complexptf_magnitude(p);
    p->re /= magnitude;
    p->im /= magnitude;
}

void complexptf_print(struct complexptf const *p) {
    char sign = p->im < 0.f ? '-' : '+';
    printf("%f %c %fi\n", p->re, sign, fabs(p->im));
}
