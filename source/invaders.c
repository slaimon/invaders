#include <stdlib.h>
#include "../include/safe.h"
#include "../include/i8080.h"

#define ROM_FILENAME "assets/INVADERS"


typedef struct shift {
    uint16_t value;
    uint8_t read_offset;
} shift_register_t;

shift_register_t shift;

#define SHIFT_INIT          \
    shift.value = 0x0000;   \
    shift.read_offset = 0   \

#define SHIFT_READ \
    shift.value & (0xFF00 >> shift.read_offset)

#define SHIFT_WRITE(x) \
    shift.value = ((x) << 8) | shift.value >> 8

#define SHIFT_SETOFFSET(x) \
    shift.read_offset = (x) & 7

// reads a value from the port
uint16_t machine_IN(uint8_t port) {
    switch (port) {
        case 3:
            return SHIFT_READ;
            break;
        
        default:
            fprintf(stderr, "ERROR! Unexpected instruction IN %02Xh\n", port);
            exit(1);
    }
}

void machine_OUT(uint8_t port, uint8_t value) {
    switch (port) {
        case 2:
            SHIFT_SETOFFSET(value);
            break;
        case 4:
            SHIFT_WRITE(value); 
            break;
        
        default:
            fprintf(stderr, "ERROR! Unexpected instruction OUT %02Xh\n", port);
            exit(1);
    }
}

void invaders_execute(i8080_t machine) {
    uint8_t opcode = machine.mem[machine.programCounter];
    uint8_t port = machine.mem[machine.programCounter+1];
    
    // IN handler
    if (opcode == 0xDB) {
        machine.A = machine_IN(port);
        machine.programCounter += 2;
    }
    // OUT handler
    else if (opcode == 0xD3) {
        machine_OUT(port, machine.A);
        machine.programCounter += 2;
    }
    else
        i8080_execute(&machine);
}

int main (void) {
    FILE* ifp = safe_fopen(ROM_FILENAME, "rb");
    bytestream_t* program = bytestream_read(ifp);
        if (program == NULL) {
        printf("No valid program found\n");
        return -1;
    }
    fclose(ifp);

    i8080_t machine;
    i8080_init(&machine);
    i8080_memory_write(&machine, *program, 0);
    bytestream_destroy(program);

    SHIFT_INIT;
    while (true) {
        invaders_execute(machine);
    }
}