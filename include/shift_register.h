#ifndef __SHIFT_REGISTER_H__
#define __SHIFT_REGISTER_H__

#include <stdint.h>

typedef struct shift_register {
    uint16_t value;
    uint8_t read_offset;
} shift_register_t;

// Initialize the shift register.
void shift_register_init(shift_register_t* shift);

// Read out of the shift register.
uint16_t shift_register_read(shift_register_t shift);

// Push a value to the shift register.
void shift_register_push(shift_register_t* shift, uint8_t x);

// Set the read offset for the shift register.
void shift_register_set_offset(shift_register_t* shift, uint8_t new_offset);

#endif