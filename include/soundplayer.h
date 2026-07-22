#include <SDL3/SDL.h>

// Some wrapper functions for playing sound effects in SDL
// Only supports .wav files

typedef struct sound {
    Uint32 length;
    Uint8* buffer;
    SDL_AudioStream* stream;
} sound_t;

typedef struct soundplayer {
    SDL_AudioDeviceID device;
    sound_t** sound;
    size_t num_sounds;
} soundplayer_t;

// Initialize the soundplayer and SDL's audio subsystem. Load all .wav files into memory.
// Each sound you load is assigned a progressive sound_id starting at 0.  
void soundplayer_init(soundplayer_t* sp, const char** fnames, const size_t num_files);

// Play a sound once.
void soundplayer_play(soundplayer_t sp, const int sound_id);

// Play a sound in loop.
// Keep calling this function as long as you need the sound playing.
// The time between function calls should be less than the length of the audio.
void soundplayer_repeat(soundplayer_t sp, const int sound_id);

// Stop playing the sound immediately.
// Will do nothing if the sound is not playing.
void soundplayer_stop(soundplayer_t sp, const int sound_id);

// Dispose of the soundplayer's resources.
void soundplayer_destroy(soundplayer_t sp);