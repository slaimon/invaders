#include <string.h>
#include "gamepad.h"

inline void gamepad_init(gamepad_t* gp) {
    memset(gp, 0, sizeof(gamepad_t));
}