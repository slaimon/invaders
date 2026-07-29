#ifndef __INVADERS_HISCORE_H__
#define __INVADERS_HISCORE_H__

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

// Players' scores for the current game.
static const uint16_t P1_SCORE_LOCATION_RAM = 0x20f8;
static const uint16_t P2_SCORE_LOCATION_RAM = 0x20fc;

// Check that the input represents two valid binary-coded decimal digits.
static bool valid_bcd(uint8_t bcd) {
    return ((bcd & 0x0f) < 9) && ((bcd >> 4) < 9);
}

// Decode the 2-byte binary-coded decimal number pointed to by score_ptr.
// Returns 0 if the pointer is invalid or if the data isn't binary-coded decimals.
static uint16_t convert_score(const uint8_t* score_ptr) {
    if (score_ptr == NULL) {
        return 0;
    }
    if (!valid_bcd(score_ptr[0]) || !valid_bcd(score_ptr[1])) {
        return 0;
    }
    uint16_t hiscore = 0;
    hiscore += 1 * (score_ptr[0] & 0x0F);
    hiscore += 10 * (score_ptr[0] >> 4);
    hiscore += 100 *  (score_ptr[1] & 0x0F);
    hiscore += 1000 * (score_ptr[1] >> 4);
    return hiscore;
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
static void hiscore_save(const i8080_t* cpu, const char* path) {
    const uint8_t* hiscore_ptr = &cpu->mem[HISCORE_LOCATION_RAM];
    const uint8_t* p1_score_ptr = &cpu->mem[P1_SCORE_LOCATION_RAM];
    const uint8_t* p2_score_ptr = &cpu->mem[P2_SCORE_LOCATION_RAM];

    uint16_t hiscore = convert_score(hiscore_ptr);
    uint16_t p1_score = convert_score(p1_score_ptr);
    uint16_t p2_score = convert_score(p2_score_ptr);

    const uint8_t* result_ptr;
    uint16_t result;
    if (hiscore > p2_score) {
        result_ptr = hiscore_ptr;
        result = hiscore;
    } else {
        result_ptr = p2_score_ptr;
        result = p2_score;
    }
    if (p1_score > result) {
        result_ptr = p1_score_ptr;
    }

    if (!SDL_SaveFile(path, result_ptr, 2)){
        SDL_Log("Failed to save high-scores file: %s", SDL_GetError());
    }
}

#endif