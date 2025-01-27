#include <stdlib.h>
#include "../include/safe.h"
#include "../include/i8080.h"
#include "../include/viewer.h"

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
    // bit 2 = Player death         SX2 2.raw
    // bit 3 = Invader death        SX3 3.raw
    // bit 4 = Extended play        SX4
    // bit 5 = Toggle amplifier     SX5
    // bit 6 = Not connected
    // bit 7 = Not connected
}

void handle_sound2(uint8_t value) {
    // TODO

    // bit 0 = Fleet movement 1     SX6 4.raw
    // bit 1 = Fleet movement 2     SX7 5.raw
    // bit 2 = Fleet movement 3     SX8 6.raw
    // bit 3 = Fleet movement 4     SX9 7.raw
    // bit 4 = UFO Hit              SX10 8.raw
    // bit 5 = Not connected (cocktail mode control, to flip the screen)
    // bit 6 = Not connected
    // bit 7 = Not connected
}

// reads a value from the port
uint16_t machine_IN(uint8_t port) {
    switch (port) {
        case 1:
            return 7<<1;
        case 2:
            return 0;
        case 3:
            return SHIFT_READ;
        
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

#define GETBIT(byte,k)  (((byte)&(1<<(k))) != 0)

#define COLOR_WHITE 0xFF
#define COLOR_GREEN 0x1C
#define COLOR_RED   0xE0
#define GETCOLOR(i,j)                                           \
    ((i >= 4 && i < 8) ? (COLOR_RED) :                          \
        ((i >= 23 && i < 30) ? (COLOR_GREEN) :                  \
            ((i >= 30 && j >= 25 && j < 136) ? (COLOR_GREEN) :  \
                (COLOR_WHITE))))

#define DISPLAY_WIDTH 224
#define DISPLAY_HEIGHT 256
uint8_t texture[DISPLAY_HEIGHT * DISPLAY_WIDTH];
void invaders_display(i8080_t* machine, viewer_t* viewer) {
    uint8_t* vram = &machine->mem[0x2400];
    
    // The video memory is arranged in 224 rows of 32 bytes. in texture space, each byte represents
    // a block 1 pixel wide and 8 pixels tall. We scan the video memory starting from the upper left
    // corner of the texture and working our way right and down, with the indices (i,j) going from
    // (0,0) to (31,W-1). For each pair (i,j) we read the corresponding byte from video memory and
    // obtain 8 different pixels from it: the one at position (8*i,j) in the texture and the other 7
    // underneath it. We get its value by multiplying its state with the color overlay.
    for (int i = 0; i < 32; ++i) {
        for(int j = 0; j < DISPLAY_WIDTH; ++j) {
            uint8_t byte = vram[32*j + 31-i];
            for(int k = 0; k < 8; ++k) {
                texture[(8*i+k)*DISPLAY_WIDTH + j] = GETBIT(byte,7-k) * GETCOLOR(i,j);
            }
        }
    }

    viewer_setFrame(viewer, texture);
}

void waitForQuitEvent(void) {
    bool quit = false;
    SDL_Event event;
    while(!quit) {
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT)
            quit = true;
    }
}

// Handle a single instruction
int invaders_execute(i8080_t* machine) {
    uint8_t opcode = machine->mem[machine->programCounter];
    uint8_t port = machine->mem[machine->programCounter+1];
    
    // IN handler
    if (opcode == 0xDB) {
        machine->A = machine_IN(port);
        machine->programCounter += 2;
        return 10;
    }
    // OUT handler
    else if (opcode == 0xD3) {
        machine_OUT(port, machine->A);
        machine->programCounter += 2;
        return 10;
    }
    else
        return i8080_execute(machine);
}

// Execute a given number of CPU cycles on machine m
#define EXECUTE_CYCLES(m, n) {          \
    size_t cycles = 0;                  \
    while (cycles < n)                  \
        cycles += invaders_execute(m);  \
}

// Handle a frame drawing cycle. A frame drawing cycle is an interval of 1/60th of a second where
// two interrupts are generated by the video hardware: the "middle-of-screen" interrupt (MSI) and
// the Vertical Blank Interrupt (VBI). The MSI generates RST 1 while the VBI generates RST 2.
void invaders_framecycle(i8080_t* machine, viewer_t* viewer) {
    size_t half_frame = 8.333*2000; // CPU cycles between two interrupts

    EXECUTE_CYCLES(machine, half_frame);
    i8080_interrupt(machine, 1);
    EXECUTE_CYCLES(machine, half_frame);
    i8080_interrupt(machine, 2);
    invaders_display(machine, viewer);
    viewer_update(viewer);
}

int main (void) {
    FILE* ifp = safe_fopen(ROM_FILENAME, "rb");
    bytestream_t* program = bytestream_read(ifp);
        if (program == NULL) {
        printf("No valid program found\n");
        return -1;
    }
    fclose(ifp);

    viewer_t viewer;
    viewer_init(&viewer, "Space Invaders", DISPLAY_WIDTH, DISPLAY_HEIGHT, 2, SDL_PIXELFORMAT_RGB332);

    i8080_t machine;
    i8080_init(&machine);
    i8080_memory_write(&machine, *program, 0);
    bytestream_destroy(program);

    SDL_Event event;

    SHIFT_INIT;
    while (true) {
        invaders_framecycle(&machine, &viewer);
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT)
            return 0;
    }
}