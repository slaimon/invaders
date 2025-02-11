#include <SDL2/SDL.h>

// Some wrapper functions for playing sound effects in SDL
// Only supports .wav files

typedef struct sound_buffer {
    Uint32 length;
    Uint8* buffer;
} sound_buffer_t;

typedef struct soundplayer {
    SDL_AudioDeviceID device;
    sound_buffer_t** sound;
    size_t num_sounds;
} soundplayer_t;

// Initialize the soundplayer and SDL's audio subsystem. Load all .wav files into memory.
// Each sound you is assigned a progressive sound_id starting at 0.  
void soundplayer_init(soundplayer_t* sp, SDL_AudioSpec spec, const char** fnames, const size_t num_files);

// Play a sound
void soundplayer_play(soundplayer_t sp, const int sound_id);