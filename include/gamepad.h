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
void gamepad_init(gamepad_t* gamepad);

// Returns a byte that represents the current state of the gamepad. This byte is
// what the Space Invaders machine expects to read when it executes an IN
// instruction. Since there are two bytes that represent the player controls
// (`input1` and `input2`), you need to tell the function which one you want.
uint8_t gamepad_getInput(gamepad_t gamepad, bool is_input1);

#endif