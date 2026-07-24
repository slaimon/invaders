#ifndef __HISCORE_H__
#define __HISCORE_H__

/*
    HEADER-ONLY

    Loading and saving high-score files for the Space Invaders machine.
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

#include "i8080.h"

// Notice that we keep track of two different locations for the hiscore:
// The one in ROM and the one in RAM. It would be useless to patch the RAM
// because it will be overwritten at startup. Likewise, the location in ROM
// is never updated by the game so it's useless to read from it.
// In simpler terms: write to ROM, read from RAM.
static const uint16_t HISCORE_LOCATION_ROM = 0x1bf4;
static const uint16_t HISCORE_LOCATION_RAM = 0x20f4;

// Check that the input represents two valid binary-coded decimal digits.
static bool valid_bcd(uint8_t bcd) {
    return ((bcd & 0x0f) < 0x9) && ((bcd >> 8) < 0x9);
}

// Load hiscores from a file into the machine's memory.
static void hiscore_load(i8080_t* cpu, const char* path) {
    size_t save_size;
    uint8_t* save = (uint8_t*) SDL_LoadFile(path, &save_size);
    if (save == NULL) {
        SDL_Log("No savefile found at %s. First time playing?", path);
        return;
    }

    bool valid =
        save_size == 2
        && valid_bcd(save[0])
        && valid_bcd(save[1]);
    if (!valid) {
        SDL_Log("Corrupted savefile! High-scores might be weird...");
    }

    cpu->mem[HISCORE_LOCATION_ROM] = save[0];
    cpu->mem[HISCORE_LOCATION_ROM + 1] = save[1];
    SDL_free(save);
}

// Save hiscores from the machine's memory to a file.
static void hiscore_save(i8080_t* cpu, const char* path) {
    uint8_t* hiscore = &cpu->mem[HISCORE_LOCATION_RAM];
    if (!SDL_SaveFile(path, hiscore, 2)){
        SDL_Log("Failed to save high-scores file: %s", SDL_GetError());
    }
}

#endif