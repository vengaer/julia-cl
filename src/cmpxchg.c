#include "cmpxchg.h"

#if CMPXCHG_LOCK_FREE == 0
#warning Hardware cmpxchg not available
#include <pthread.h>

static pthread_mutex_t lock;

bool cmpxchg_init(void) {
    return pthread_mutex_init(&lock, NULL) == 0;
}

bool cmpxchg_cleanup(void) {
    return pthread_mutex_destroy(&lock) == 0;
}

int32_t cmpxchg32(int32_t volatile *addr, int32_t oldv, int32_t newv) {
    pthread_mutex_lock(&lock);
    int32_t o = *addr;

    if(o == oldv) {
        *addr = newv;
    }
    pthread_mutex_unlock(&lock);
    return o;
}

uint32_t cmpxchgu32(uint32_t volatile *addr, uint32_t oldv, uint32_t newv) {
    pthread_mutex_lock(&lock);
    uint32_t o = *addr;

    if(o == oldv) {
        *addr = newv;
    }
    pthread_mutex_unlock(&lock);
    return o;
}
    

#endif
