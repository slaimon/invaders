#include <SDL2/SDL.h>

// Some wrapper functions for playing sound effects in SDL
// Only supports .wav files

typedef struct sound_buffer {
    Uint32 length;
    Uint8* buffer;
} sound_buffer_t;

typedef struct soundplayer {
    SDL_AudioDeviceID device;
    sound_buffer_t* sound;
    size_t num_sounds;
} soundplayer_t;

// Load a sound from a .wav file. Each sound you load is assigned a progressive sound_id starting at 0.  
void soundplayer_load_file(soundplayer_t sp, char* fname);

// Prepare the sound player for playback. Call this function once before you start using it.
void soundplayer_activate(soundplayer_t sp);

// Play a sound
void soundplayer_play(soundplayer_t sp, int sound_id);