#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

/*
    HEADER-ONLY

    The Space Invaders gamepad and control mappings.
*/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "flags.h"

#define KEY_COIN        SDLK_C
#define KEY_TILT        SDLK_DELETE
#define KEY_P1START     SDLK_1
#define KEY_P1FIRE      SDLK_UP
#define KEY_P1LEFT      SDLK_LEFT
#define KEY_P1RIGHT     SDLK_RIGHT
#define KEY_P2START     SDLK_2
#define KEY_P2FIRE      SDLK_W
#define KEY_P2LEFT      SDLK_A
#define KEY_P2RIGHT     SDLK_D

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
static void gamepad_init(gamepad_t* gamepad) {
    memset(gamepad, 0, sizeof(gamepad_t));
}

// Returns a byte that represents the current state of the gamepad. This byte is
// what the Space Invaders machine expects to read when it executes an IN
// instruction. Since there are two bytes that represent the player controls
// (`input1` and `input2`), you need to tell the function which one you want.
static uint8_t gamepad_getInput(gamepad_t gamepad, bool is_input1) {
    uint8_t result = 0;
    
    if (is_input1) {
        flag_set(0, &result, gamepad.coin);
        flag_set(1, &result, gamepad.p2_start);
        flag_set(2, &result, gamepad.p1_start);
        flag_set(4, &result, gamepad.p1_fire);
        flag_set(5, &result, gamepad.p1_left);
        flag_set(6, &result, gamepad.p1_right);
    } else {
        // bit 0 = DIP3 00 = 3 ships  10 = 5 ships
        // bit 1 = DIP5 01 = 4 ships  11 = 6 ships
        // bit 2 = Tilt
        // bit 3 = DIP6 0 = extra ship at 1500, 1 = extra ship at 1000
        // bit 4 = P2 shot (1 if pressed)
        // bit 5 = P2 left (1 if pressed)
        // bit 6 = P2 right (1 if pressed)
        // bit 7 = DIP7 Coin info displayed in demo screen 0=ON
        
        flag_set(2, &result, gamepad.tilt);
    }
    
    return result;       
}

#endif