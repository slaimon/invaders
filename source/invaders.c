#include <stdlib.h>
#include "../include/safe.h"
#include "../include/i8080.h"
#include "../include/viewer.h"

#define ROM_FILENAME "assets/INVADERS"

// Control mapping
#define KEY_COIN    SDL_SCANCODE_C
#define KEY_P1START SDL_SCANCODE_1
#define KEY_P1FIRE  SDL_SCANCODE_UP
#define KEY_P1LEFT  SDL_SCANCODE_LEFT
#define KEY_P1RIGHT SDL_SCANCODE_RIGHT
#define KEY_P2START SDL_SCANCODE_2
#define KEY_P2FIRE  SDL_SCANCODE_W
#define KEY_P2LEFT  SDL_SCANCODE_A
#define KEY_P2RIGHT SDL_SCANCODE_D

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
    ((shift.value >> (8-shift.read_offset)) & 0xFF)

// Push a value to the shift register
#define SHIFT_WRITE(x) \
    shift.value = ((x) << 8) | (shift.value >> 8)

// Set the read offset for the shift register
#define SHIFT_SETOFFSET(x) \
    shift.read_offset = (x) & 7


#define GETBIT(byte,k)      (bool)((byte)&(1<<(k)))
#define SETBIT(byte,k)      ((byte)|(1<<(k)))
#define CLEARBIT(byte,k)    ((byte)&!(1<<(k)))

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

uint8_t input1 = 0;
uint8_t input2 = 0;
const Uint8* keystate;

#define UPDATE_INPUT(key, input, bit) \
    if (keystate[key])                \
        input = SETBIT(input,bit);    \
    else                              \
        input = CLEARBIT(input,bit)

// returns true if program should quit
bool process_input(void) {
    SDL_Event event;
    SDL_PumpEvents();

    if (keystate[KEY_COIN] && !GETBIT(input1,0))
        input1 = SETBIT(input1,0);
    else
        input1 = CLEARBIT(input1,0);
    UPDATE_INPUT(KEY_P1START, input1, 2);
    UPDATE_INPUT(KEY_P1FIRE, input1, 4);
    UPDATE_INPUT(KEY_P1LEFT, input1, 5);
    UPDATE_INPUT(KEY_P1RIGHT, input1, 6);

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            return true;
        if (event.type == SDL_KEYDOWN)
            if (event.key.keysym.sym == SDLK_ESCAPE)
                return true;
    }

    return false;
}

// reads a value from the port
uint16_t machine_IN(uint8_t port) {
    switch (port) {
        case 0: // never used
            return 0;
        case 1:
            return input1;
        case 2:
            return input2;
        case 3: // read shift register
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

// Execute instructions until the cycles budget is spent
void invaders_execute_cycles(i8080_t* machine, size_t max_cycles) {
    uint8_t opcode;
    uint8_t port;

    size_t cycles = 0;
    while(cycles <= max_cycles) {
        opcode = machine->mem[machine->programCounter];
        port = machine->mem[machine->programCounter+1];

        // IN handler
        if (opcode == 0xDB) {
            machine->A = machine_IN(port);
            machine->programCounter += 2;
            cycles += 10;
        }
        // OUT handler
        else if (opcode == 0xD3) {
            machine_OUT(port, machine->A);
            machine->programCounter += 2;
            cycles += 10;
        }
        else
            cycles += i8080_execute(machine);
    }
}

// Execute a frame drawing loop. A frame drawing loop is an interval of 1/60th of a second where
// two interrupts are generated by the video hardware: the "middle-of-screen" interrupt (MSI) and
// the Vertical Blank Interrupt (VBI). The MSI generates RST 1 while the VBI generates RST 2.
void invaders_frame(i8080_t* machine, viewer_t* viewer) {
    size_t half_frame = 8.333*2000; // CPU cycles between two interrupts

    invaders_execute_cycles(machine, half_frame);
    i8080_interrupt(machine, 1);
    invaders_execute_cycles(machine, half_frame);
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

    keystate = SDL_GetKeyboardState(NULL);

    SHIFT_INIT;
    machine.mem[0x20EB] = 1;
    while (true) {
        invaders_frame(&machine, &viewer);
        if (process_input())
            break;
    }

    return 0;
}