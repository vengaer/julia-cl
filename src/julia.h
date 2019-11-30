#ifndef JULIA_H
#define JULIA_H

#include "complexpt.h"

#include <stdbool.h>

#include <CL/cl.h>

#define JULIA_RADIUS 2.f

bool julia_init(unsigned iters);
void julia_cleanup(void);

bool julia_update_dims(cl_uint width, cl_uint height);
bool julia_update_constant(struct complexptf const *c);
bool julia_run_kernel(unsigned char *out);


#endif
