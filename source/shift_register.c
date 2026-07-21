#include "shift_register.h"

inline void shift_register_init(shift_register_t* shift) {
    shift->value = 0x0000;
    shift->read_offset = 0;
}

inline uint16_t shift_register_read(shift_register_t shift) {
    return ((shift.value >> (8-shift.read_offset)) & 0xFF);
}

inline void shift_register_push(shift_register_t* shift, uint8_t x) {
    shift->value = (((uint16_t) x) << 8) | (shift->value >> 8);
}

inline void shift_register_set_offset(shift_register_t* shift, uint8_t new_offset) {
    shift->read_offset = new_offset & 7;
}