#ifndef PARTICLE_H
#define PARTICLE_H

#include "complexpt.h"

#include <stdbool.h>

bool particle_spawn(void);
bool particle_join(void);

struct complexptf particle_position(void);

#endif
