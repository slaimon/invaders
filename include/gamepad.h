#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

#include <stdbool.h>

typedef struct gamepad {
    bool coin;
    bool tilt;

    bool p1_start;
    bool p1_fire;
    bool p1_left;
    bool p1_right;

    bool p2_start;
    bool p2_fire;
    bool p2_left;
    bool p2_right;
} gamepad_t;

// Initialize the gamepad, no buttons are pressed
void gamepad_init(gamepad_t* gp);

#endif