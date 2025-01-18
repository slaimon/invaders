#include "../include/safe.h"
#include "../include/i8080.h"

#define ROM_FILENAME "assets/INVADERS"

typedef struct shift {
    uint16_t value;
    uint8_t read_offset;
} shift_register_t;

shift_register_t shift;

uint8_t shift_read(void) {
    uint16_t mask = 0xFF00 >> shift.read_offset;
    return shift.value & mask;
}

void shift_write(uint8_t x) {
    shift.value = (x << 8) | shift.value >> 8;
}

void shift_setOffset(uint8_t off) {
    shift.read_offset = off & 7;
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

    while (true) {
        i8080_execute(&machine);

        if (machine.programCounter == 0x0ADD &&
            machine.A != 0) {
                printf("Infinite loop reached. Success!\n");
                return 0;
            }
    }
}