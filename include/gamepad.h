#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

/*
    HEADER-ONLY

    The Space Invaders gamepad and control mappings.
*/

#include <SDL3/SDL.h>
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
    uint8_t flag = 0;
    
    if (is_input1) {
        flag_set(0, &flag, gamepad.coin);
        flag_set(1, &flag, gamepad.p2_start);
        flag_set(2, &flag, gamepad.p1_start);
        flag_set(4, &flag, gamepad.p1_fire);
        flag_set(5, &flag, gamepad.p1_left);
        flag_set(6, &flag, gamepad.p1_right);
    } else {
        // bit 0 = DIP3 00 = 3 ships  10 = 5 ships
        // bit 1 = DIP5 01 = 4 ships  11 = 6 ships
        // bit 2 = Tilt
        // bit 3 = DIP6 0 = extra ship at 1500, 1 = extra ship at 1000
        // bit 4 = P2 shot (1 if pressed)
        // bit 5 = P2 left (1 if pressed)
        // bit 6 = P2 right (1 if pressed)
        // bit 7 = DIP7 Coin info displayed in demo screen 0=ON
        
        flag_set(4, &flag, gamepad.p2_fire);
        flag_set(5, &flag, gamepad.p2_left);
        flag_set(6, &flag, gamepad.p2_right);
        flag_set(2, &flag, gamepad.tilt);
    }
    
    return flag;       
}

static void gamepad_handle_event(gamepad_t* gamepad, SDL_Event* event) {
    bool new_state;

    if (event->type == SDL_EVENT_KEY_DOWN)
        new_state = true;
    else if (event->type == SDL_EVENT_KEY_UP)
        new_state = false;
    else
        return;
    
    switch (event->key.key) {
        case KEY_COIN:
            gamepad->coin = new_state;
            return;
        case KEY_TILT:
            gamepad->tilt = new_state;
            return;
        case KEY_P1START:
            gamepad->p1_start = new_state;
            return;
        case KEY_P2START:
            gamepad->p2_start = new_state;
            return;
        
        // Since the two players never use the gamepad at the same time, it's more
        // practical to make it so that each player can use either set of controls
        case KEY_P1FIRE:
        case KEY_P2FIRE:
            gamepad->p1_fire = new_state;
            gamepad->p2_fire = new_state;
            return;
        case KEY_P1LEFT:
        case KEY_P2LEFT:
            gamepad->p2_left = new_state;
            gamepad->p1_left = new_state;
            return;
        case KEY_P1RIGHT:
        case KEY_P2RIGHT:
            gamepad->p1_right = new_state;
            gamepad->p2_right = new_state;
            return;
        
        default:
            return;
    }
}

#endif