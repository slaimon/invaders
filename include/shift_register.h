#ifndef __SHIFT_REGISTER_H__
#define __SHIFT_REGISTER_H__

/*
	HEADER-ONLY

    The shift register is a hardware resource on the original Space Invaders cabinet board.
    It allows the machine to move sprites on the screen without burning precious CPU time.

    The following description is taken from the webpage:
    https://www.computerarcheology.com/Arcade/SpaceInvaders/Hardware.html

		16 bit shift register:

		f              0	bit
		xxxxxxxxyyyyyyyy
		
		Writing to port 4 shifts x into y, and the new value into x, eg.
		$0000,
		write $aa -> $aa00,
		write $ff -> $ffaa,
		write $12 -> $12ff, ..
		
		Writing to port 2 (bits 0,1,2) sets the offset for the 8 bit result, eg.
		offset 0:
		rrrrrrrr		result=xxxxxxxx
		xxxxxxxxyyyyyyyy
		
		offset 2:
		rrrrrrrr	result=xxxxxxyy
		xxxxxxxxyyyyyyyy
		
		offset 7:
			rrrrrrrr	result=xyyyyyyy
		xxxxxxxxyyyyyyyy
		
		Reading from port 3 returns said result.
*/

#include <stdint.h>

typedef struct shift_register {
    uint16_t value;
    uint8_t read_offset;
} shift_register_t;

// Initialize the shift register.
static void shift_register_init(shift_register_t* shift) {
    shift->value = 0x0000;
    shift->read_offset = 0;
}

// Read out of the shift register.
static uint16_t shift_register_read(shift_register_t shift) {
    return ((shift.value >> (8-shift.read_offset)) & 0xFF);
}

// Push a value to the shift register.
static void shift_register_push(shift_register_t* shift, uint8_t x) {
    shift->value = (((uint16_t) x) << 8) | (shift->value >> 8);
}

// Set the read offset for the shift register.
static void shift_register_set_offset(shift_register_t* shift, uint8_t new_offset) {
    shift->read_offset = new_offset & 7;
}

#endif