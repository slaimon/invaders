#include <string.h>
#include "gamepad.h"
#include "flags.h"

inline void gamepad_init(gamepad_t* gamepad) {
    memset(gamepad, 0, sizeof(gamepad_t));
}

uint8_t getInput(gamepad_t gamepad, bool is_input1) {
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