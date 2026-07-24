#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include <stdarg.h>
#include <string.h>
#include "i8080.h"
#include "flags.h"

// Video and audio devices
#include "viewer.h"
#include "soundplayer.h"

// Shift register hardware and player controls
#include "shift_register.h"
#include "gamepad.h"

// Savefiles
#include "hiscore.h"
static char* save_path;

// Virtual devices
static i8080_t cpu;
static shift_register_t shift;
static gamepad_t gamepad;
static soundplayer_t soundplayer;
static viewer_t viewer;

// Asset files names and paths
static const char* ROM_PATH = "data/INVADERS";
static const char* SAVE_PATH = "data/score.b";
static const char* SOUND_DIR = "data/sound";
static const char* sfx_files[] = {
    "player_shoot.wav",
    "player_death.wav",
    "step1.wav",
    "step2.wav",
    "step3.wav",
    "step4.wav",
    "invader_death.wav",
    "ufo_flying.wav",
    "ufo_hit.wav"
};

// Sound effect identifiers
typedef enum sfx {
    PLAYER_SHOOT,
    PLAYER_DEATH,
    STEP1,
    STEP2,
    STEP3,
    STEP4,
    INVADER_DEATH,
    UFO_FLYING,
    UFO_HIT,

    NUMBER_OF_SFX
} sfx_t;

// These store the old values of the gamestate bytes that control sound.
static uint8_t sound1 = 0;
static uint8_t sound2 = 0;

// Check the kth bit of both `curr` and `prev`. If a rising edge is detected
// (i.e. the bit is on in `curr` and off `prev`), the sound corresponding to
// `sound_id` is played.
// In every case, `prev` is overwritten with the value of `curr`.
static void play_sound(int sound_id, uint8_t k, uint8_t* prev, uint8_t curr) {
    bool curr_flag = flag_get(k, curr);

    if (curr_flag && !flag_get(k, *prev)) {
        soundplayer_play(soundplayer, sound_id);
    }

    flag_set(k, prev, curr_flag);
}

// Plays sound effects according to the game state.
// Since there are two gamestate bytes that control sound,
// you need to tell the function which one you want to use.
static void handle_sound(uint8_t curr, bool is_sound1) {
    if (is_sound1) {
        uint8_t* prev = &sound1;

        if (flag_get(0, curr)) {
            soundplayer_repeat(soundplayer, UFO_FLYING);
            flag_set(0, prev, true);
        }
        else if (flag_get(0, *prev)) {
            soundplayer_stop(soundplayer, UFO_FLYING);
            flag_set(0, prev, false);
        }
    
        play_sound(PLAYER_SHOOT, 1, prev, curr);
        play_sound(PLAYER_DEATH, 2, prev, curr);
        play_sound(INVADER_DEATH, 3, prev, curr);
    } else {
        uint8_t* prev = &sound2;
        
        play_sound(STEP1, 0, prev, curr);
        play_sound(STEP2, 1, prev, curr);
        play_sound(STEP3, 2, prev, curr);
        play_sound(STEP4, 3, prev, curr);
        play_sound(UFO_HIT, 4, prev, curr);
    }
}

// Reads a value from the port
static uint16_t handle_IN(uint8_t port) {
    switch (port) {
        case 0: // never used
            return 0;
        case 1:
            return gamepad_getInput(gamepad, true);
        case 2:
            return gamepad_getInput(gamepad, false);
        case 3:
            return shift_register_read(shift);
        
        default:
            SDL_Log("ERROR! Unexpected instruction IN %02Xh\n", port);
            SDL_AppQuit(NULL, SDL_APP_FAILURE);
            return 0; // unreachable
    }
}

// Writes a value to the port
static void handle_OUT(uint8_t port, uint8_t value) {
    switch (port) {
        case 2:
            shift_register_set_offset(&shift, value);
            break;
        case 3:
            handle_sound(value, true);
            break;
        case 4:
            shift_register_push(&shift, value);
            break;
        case 5:
            handle_sound(value, false);
            break;
        case 6:
            // watchdog port:
            // "The watchdog checks to see if the system has crashed. If the watchdog
            // doesn't receive a read/write request after a certain number of clock cycles
            // it resets the whole machine."
            //   - akira1310 on reddit
            break;
        
        default:
            SDL_Log("ERROR! Unexpected instruction OUT %02Xh\n", port);
            SDL_AppQuit(NULL, SDL_APP_FAILURE);
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
static uint8_t texture[DISPLAY_HEIGHT * DISPLAY_WIDTH];

// Update the viewer with the machine's state
static void update_viewer() {
    uint8_t* vram = &cpu.mem[0x2400];
    
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
                texture[(8*i+k)*DISPLAY_WIDTH + j] = flag_get(7-k, byte) ? GETCOLOR(i,j) : 0;
            }
        }
    }

    viewer_setFrame(&viewer, texture);
    viewer_update(&viewer);
}

// Execute instructions until the cycles budget is spent
static void execute_cycles(size_t max_cycles) {
    uint8_t opcode;
    uint8_t port;

    size_t cycles = 0;
    while(cycles <= max_cycles) {
        opcode = cpu.mem[cpu.programCounter];
        port = cpu.mem[cpu.programCounter+1];

        // IN handler
        if (opcode == 0xDB) {
            cpu.A = handle_IN(port);
            cpu.programCounter += 2;
            cycles += 10;
        }
        // OUT handler
        else if (opcode == 0xD3) {
            handle_OUT(port, cpu.A);
            cpu.programCounter += 2;
            cycles += 10;
        }
        else
            cycles += i8080_execute(&cpu);
    }
}

// Write a file-system path to the variable `dest` by concatenating `n` names.
// The previous content of the variable `dest` is overwritten.
// E.g.: `write_path(dest, 3, "folder", "subdir", "file.txt")` will place in
// `dest` the string: `folder/subdir/file.txt`.
static void write_path(char* dest, int n, ...) {
    va_list args;
    va_start(args, n);
    strcpy(dest, va_arg(args, char*));
    for (int i = 0; i < n-1; i++) {
        strcat(dest, "/");
        strcat(dest, va_arg(args, char*));
    }
    va_end(args);
}

static void sfx_init(soundplayer_t* sp, const char* sound_dir) {
    char** fpaths = SDL_malloc(NUMBER_OF_SFX * sizeof(char*));
    for (int i = 0; i < NUMBER_OF_SFX; i++) {
        fpaths[i] = SDL_malloc(strlen(sound_dir) + strlen(sfx_files[i]) + 3);
        write_path(fpaths[i], 2, sound_dir, sfx_files[i]);
    }
    const char** fpaths_ = (const char**) fpaths;
    soundplayer_init(sp, fpaths_, NUMBER_OF_SFX);

    for (int i = 0; i < NUMBER_OF_SFX; i++)
        SDL_free(fpaths[i]);
    SDL_free(fpaths_);

    // adjust relative sfx volume
    soundplayer_set_gain(*sp, PLAYER_SHOOT, 0.4);
    soundplayer_set_gain(*sp, INVADER_DEATH, 0.4);
    soundplayer_set_gain(*sp, UFO_FLYING, 0.2);
    soundplayer_set_gain(*sp, UFO_HIT, 0.2);
}

static void print_usage(const char* program_name) {
    printf("usage: %s [assets_dir]\n", program_name);
    printf("Play Space Invaders!\n");
    printf("Optionally provide a path to the assets folder (the default location is .)\n");
}

static bool cleanup_viewer = false;
static bool cleanup_sound = false;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
    const char* base_dir;

    if (argc == 1) {
        base_dir = SDL_GetBasePath();
    } else if (argc == 2) {
        base_dir = argv[1];
    } else {
        print_usage(argv[0]);
        return SDL_APP_FAILURE;
    }

    // TODO: use SDL functions instead of libc or safe.h e.g. SDL_free, SDL_malloc etc.
    // Note that libraries like bytestream and i8080 also use libc and safe.h

    // obtain asset file paths
    char* rom_path = SDL_malloc(strlen(base_dir) + strlen(ROM_PATH) + 2);
    write_path(rom_path, 2, base_dir, ROM_PATH);
    char* sound_dir = SDL_malloc(strlen(base_dir) + strlen(SOUND_DIR) + 2);
    write_path(sound_dir, 2, base_dir, SOUND_DIR);
    save_path = SDL_malloc(strlen(base_dir) + strlen(SAVE_PATH) + 2);
    write_path(save_path, 2, base_dir, SAVE_PATH);

    // read the ROM file
    size_t rom_size;
    void* rom = SDL_LoadFile(rom_path, &rom_size);
    SDL_free(rom_path);
    if (rom == NULL) {
        SDL_Log("Error loading ROM file: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // initialize the CPU
    i8080_init(&cpu);
    bytestream_t program;
    program.data = rom;
    program.size = rom_size;
    i8080_memory_write(&cpu, program, 0);
    SDL_free(program.data);
    hiscore_load(&cpu, save_path);

    // initialize other devices
    viewer_init(&viewer, "Space Invaders", DISPLAY_WIDTH, DISPLAY_HEIGHT, 2, SDL_PIXELFORMAT_RGB332);
    cleanup_viewer = true;
    sfx_init(&soundplayer, sound_dir);
    SDL_free(sound_dir);
    cleanup_sound = true;
    shift_register_init(&shift);
    gamepad_init(&gamepad);


    return SDL_APP_CONTINUE;
}

// Polls for all events in the SDL queue.
// Returns true if the program should quit.
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;
    if (event->type == SDL_EVENT_KEY_UP)
        if (event->key.key == SDLK_ESCAPE)
            return SDL_APP_SUCCESS;
    
    bool new_state;

    if (event->type == SDL_EVENT_KEY_DOWN)
        new_state = true;
    else if (event->type == SDL_EVENT_KEY_UP)
        new_state = false;
    else
        return SDL_APP_CONTINUE;
    
    switch (event->key.key) {
        case KEY_COIN:
            gamepad.coin = new_state;
            break;
        case KEY_TILT:
            gamepad.tilt = new_state;
            break;
        case KEY_P1START:
            gamepad.p1_start = new_state;
            break;
        case KEY_P1FIRE:
            gamepad.p1_fire = new_state;
            break;
        case KEY_P1LEFT:
            gamepad.p1_left = new_state;
            break;
        case KEY_P1RIGHT:
            gamepad.p1_right = new_state;
            break;
        case KEY_P2START:
            gamepad.p2_start = new_state;
            break;
        case KEY_P2FIRE:
            gamepad.p2_fire = new_state;
            break;
        case KEY_P2LEFT:
            gamepad.p2_left = new_state;
            break;
        case KEY_P2RIGHT:
            gamepad.p2_right = new_state;
            break;
        
        default:
            return SDL_APP_CONTINUE;
    }

    return SDL_APP_CONTINUE;
}

// Execute a frame drawing loop. A frame drawing loop is an interval of 1/60th of a second where
// two interrupts are generated by the video hardware: the "middle-of-screen" interrupt (MSI) and
// the Vertical Blank Interrupt (VBI). The MSI generates RST 1 while the VBI generates RST 2.
SDL_AppResult SDL_AppIterate(void* appstate) {
    // CPU cycles between two interrupts
    size_t half_frame = 8.333*2000;

    // emulate one whole frame
    execute_cycles(half_frame);
    i8080_interrupt(&cpu, 1);
    execute_cycles(half_frame);
    i8080_interrupt(&cpu, 2);

    update_viewer();
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    if (cleanup_viewer)
        viewer_destroy(viewer);
    if (cleanup_sound)
        soundplayer_destroy(soundplayer);

    hiscore_save(&cpu, save_path);
    SDL_free(save_path);

    SDL_Quit();
}