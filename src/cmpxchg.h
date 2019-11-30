#ifndef CMPXCHG_H
#define CMPXCHG_H

#include <stdint.h>

#if defined i386 || defined __i386__ || defined __x86_64__

#define CMPXCHG_ASM(addr, oldv, newv)                       \
    asm volatile ("lock; cmpxchg %2, %1"                    \
                  : "=a"((oldv))                            \
                  : "m"(*(addr)), "r"((newv)), "a"((oldv))  \
                  : "memory" )

static inline int32_t cmpxchg32(int32_t volatile *addr, int32_t oldv, int32_t newv) {
    CMPXCHG_ASM(addr, oldv, newv);
    return oldv;
}

static inline uint32_t cmpxchgu32(uint32_t volatile *addr, uint32_t oldv, uint32_t newv) {
    CMPXCHG_ASM(addr, oldv, newv);
    return oldv;
}

static inline void atomic_write32(int32_t volatile *addr, uint32_t v) {
    int32_t old;
    do {
        old = *addr;
    } while(cmpxchg32(addr, old, v) != old);
}

static inline void atomic_writeu32(uint32_t volatile *addr, uint32_t v) {
    uint32_t old;
    do {
        old = *addr;
    } while(cmpxchgu32(addr, old, v) != old);
}

#else
#error Hardware cmpxchg not available
#endif

#endif
