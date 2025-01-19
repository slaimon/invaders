#include <stdlib.h>
#include "../include/safe.h"
#include "../include/i8080.h"

#define ROM_FILENAME "assets/INVADERS"


typedef struct shift {
    uint16_t value;
    uint8_t read_offset;
} shift_register_t;

shift_register_t shift;

// Initialize the shift register
#define SHIFT_INIT          \
    shift.value = 0x0000;   \
    shift.read_offset = 0   \

// Read out of the shift register
#define SHIFT_READ \
    shift.value & (0xFF00 >> shift.read_offset)

// Push a value to the shift register
#define SHIFT_WRITE(x) \
    shift.value = ((x) << 8) | shift.value >> 8

// Set the read offset for the shift register
#define SHIFT_SETOFFSET(x) \
    shift.read_offset = (x) & 7


void handle_sound1(uint8_t value) {
    // TODO

    // bit 0 = UFO (repeats)        SX0 0.raw
    // bit 1 = Shot                 SX1 1.raw
    // bit 2 = Flash (player die)   SX2 2.raw
    // bit 3 = Invader die          SX3 3.raw
    // bit 4 = Extended play        SX4
    // bit 5 = AMP enable           SX5
    // bit 6 = not connected
    // bit 7 = not connected
}

void handle_sound2(uint8_t value) {
    // TODO

    // bit 0 = Fleet movement 1     SX6 4.raw
    // bit 1 = Fleet movement 2     SX7 5.raw
    // bit 2 = Fleet movement 3     SX8 6.raw
    // bit 3 = Fleet movement 4     SX9 7.raw
    // bit 4 = UFO Hit              SX10 8.raw
    // bit 5 = not connected (cocktail mode control, to flip the screen)
    // bit 6 = not connected
    // bit 7 = not connected
}

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
        case 3:
            handle_sound1(value);
            break;
        case 4:
            SHIFT_WRITE(value); 
            break;
        case 5:
            handle_sound2(value);
            break;
        case 6:
            // watchdog port:
            // "The watchdog checks to see if the system has crashed. If the watchdog
            // doesn't receive a read/write request after a certain number of clock cycles
            // it resets the whole machine."
            //   - akira1310 on reddit
            break;
        
        default:
            fprintf(stderr, "ERROR! Unexpected instruction OUT %02Xh\n", port);
            exit(1);
    }
}

void invaders_execute(i8080_t* machine) {
    uint8_t opcode = machine->mem[machine->programCounter];
    uint8_t port = machine->mem[machine->programCounter+1];
    
    // IN handler
    if (opcode == 0xDB) {
        machine->A = machine_IN(port);
        machine->programCounter += 2;
    }
    // OUT handler
    else if (opcode == 0xD3) {
        machine_OUT(port, machine->A);
        machine->programCounter += 2;
    }
    else
        i8080_execute(machine);
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
        invaders_execute(&machine);
        if (machine.programCounter == 0x0ADD &&
            machine.A != 0) {
                printf("Infinite loop reached. Success!\n");
                return 0;
            }
    }
}