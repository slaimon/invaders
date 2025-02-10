#include <stdlib.h>
#include "../include/safe.h"
#include "../include/i8080.h"
#include "../include/viewer.h"

#define ROM_FILENAME "assets/INVADERS"

// Control mapping
#define KEY_COIN    SDLK_c
#define KEY_TILT    SDLK_DELETE
#define KEY_P1START SDLK_1
#define KEY_P1FIRE  SDLK_UP
#define KEY_P1LEFT  SDLK_LEFT
#define KEY_P1RIGHT SDLK_RIGHT
#define KEY_P2START SDLK_2
#define KEY_P2FIRE  SDLK_w
#define KEY_P2LEFT  SDLK_a
#define KEY_P2RIGHT SDLK_d

typedef struct shift_register {
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

typedef struct key_states {
    bool coin;
    bool tilt;

    bool p1_start;
    bool p1_fire;
    bool p1_left;
    bool p1_right;

    bool p2_start;
    bool p2_fire;
    bool p2_left;
    bool p2_right;
} key_states_t;

key_states_t keystates;

// Initialize the keyboard state
#define KEYSTATES_INIT      \
    memset(&keystates, 0, sizeof(key_states_t))

// Encode a boolean condition into the kth bit of byte
#define WRITE_TO_BIT(byte, state, k)   \
    if(state) byte |= (1<<(k)); else byte &= ~(1<<(k))

void print_key_states() {
    printf("%d %d %d %d %d\n",
        keystates.coin,
        keystates.p1_start,
        keystates.p1_fire,
        keystates.p1_left,
        keystates.p1_right
    );
}

uint8_t getInput1(void) {
    uint8_t result = 0;

    // bit 0 = CREDIT (1 if deposit)
    // bit 1 = 2P start (1 if pressed)
    // bit 2 = 1P start (1 if pressed)
    // bit 3 = Always 1
    // bit 4 = 1P shot (1 if pressed)
    // bit 5 = 1P left (1 if pressed)
    // bit 6 = 1P right (1 if pressed)
    // bit 7 = Not connected
    
    WRITE_TO_BIT(result, keystates.coin, 0);
    WRITE_TO_BIT(result, keystates.p2_start, 1);
    WRITE_TO_BIT(result, keystates.p1_start, 2);
    WRITE_TO_BIT(result, keystates.p1_fire, 4);
    WRITE_TO_BIT(result, keystates.p1_left, 5);
    WRITE_TO_BIT(result, keystates.p1_right, 6);
    
    return result;       
}

uint8_t getInput2(void) {
    uint8_t result = 0;

    // bit 0 = DIP3 00 = 3 ships  10 = 5 ships
    // bit 1 = DIP5 01 = 4 ships  11 = 6 ships
    // bit 2 = Tilt
    // bit 3 = DIP6 0 = extra ship at 1500, 1 = extra ship at 1000
    // bit 4 = P2 shot (1 if pressed)
    // bit 5 = P2 left (1 if pressed)
    // bit 6 = P2 right (1 if pressed)
    // bit 7 = DIP7 Coin info displayed in demo screen 0=ON

    WRITE_TO_BIT(result, keystates.tilt, 2);

    return result;
}

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

// Given a keyboard event, use it to update the program's state
void handle_keyboard_event(SDL_Event event) {
    if (event.key.repeat)
        return;

    bool new_state;
    if (event.type == SDL_KEYDOWN)
        new_state = true;
    else if (event.type == SDL_KEYUP)
        new_state = false;
    else
        return;
    
    switch (event.key.keysym.sym) {
        case KEY_COIN:
            keystates.coin = new_state;
            break;
        case KEY_TILT:
            keystates.tilt = new_state;
            break;
        case KEY_P1START:
            keystates.p1_start = new_state;
            break;
        case KEY_P1FIRE:
            keystates.p1_fire = new_state;
            break;
        case KEY_P1LEFT:
            keystates.p1_left = new_state;
            break;
        case KEY_P1RIGHT:
            keystates.p1_right = new_state;
            break;
        
        default:
            return;
    }
}

// Polls for all events in the SDL queue.
// Returns true if the program should quit.
bool process_events(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            return true;
        if (event.type == SDL_KEYDOWN)
            if (event.key.keysym.sym == SDLK_ESCAPE)
                return true;
        
        handle_keyboard_event(event);
    }

    return false;
}

// Reads a value from the port
uint16_t machine_IN(uint8_t port) {
    switch (port) {
        case 0: // never used
            return 0;
        case 1:
            return getInput1();
        case 2:
            return getInput2();
        case 3: // read shift register
            return SHIFT_READ;
        
        default:
            fprintf(stderr, "ERROR! Unexpected instruction IN %02Xh\n", port);
            exit(1);
    }
}

// Writes a value to the port
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

#define GETBIT(byte,k) \
    (bool) ((byte)&(1<<(k)))

#define DISPLAY_WIDTH 224
#define DISPLAY_HEIGHT 256
uint8_t texture[DISPLAY_HEIGHT * DISPLAY_WIDTH];

// Update the viewer with the machine's state
void invaders_display(i8080_t* machine, viewer_t* viewer) {
    uint8_t* vram = &machine->mem[0x2400];
    /*
        The video memory is arranged in 224 rows of 32 bytes. in texture space, each byte represents
        a block 1 pixel wide and 8 pixels tall. We scan the video memory starting from the upper left
        corner of the texture and working our way right and down, with the indices (i,j) going from
        (0,0) to (31,W-1). For each pair (i,j) we read the corresponding byte from video memory and
        obtain 8 different pixels from it: the one at position (8*i,j) in the texture and the other 7
        underneath it. We get its value by multiplying its state with the color overlay.
    */
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

    // emulate one whole frame
    invaders_execute_cycles(machine, half_frame);
    i8080_interrupt(machine, 1);
    invaders_execute_cycles(machine, half_frame);
    i8080_interrupt(machine, 2);

    // update the view
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

    SHIFT_INIT;
    KEYSTATES_INIT;
    while (true) {
        invaders_frame(&machine, &viewer);
        if (process_events())
            break;
    }

    return 0;
}