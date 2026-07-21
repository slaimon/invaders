#ifndef __BITFLAGS_H__
#define __BITFLAGS_H__

/*
    Yes, this header file contains implementations. Oh no!

    It's actually quite harmless and allows the compiler to inline calls to
    these pervasive functions, avoiding a significant amount of overhead.

    In theory, at least.
*/

#include <stdbool.h>
#include <stdint.h>

// Set the kth bit of `flag` to `new_state`.
static void flag_set(uint8_t k, uint8_t* flag, bool new_state) {
    if(new_state)
        *flag |= (1<<(k));
    else
        *flag &= ~(1<<(k));
}

// Get the kth bit of `flag`.
static bool flag_get(uint8_t k, uint8_t flag) {
    return (bool) ( flag & (1<<k));
}

#endif