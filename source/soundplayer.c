#include <SDL3/SDL_audio.h>
#include <stdlib.h>

#include "safe.h"
#include "soundplayer.h"

/// Tries to load the given sound file and store it at sound_ptr.
/// If the loading process fails, the function returns false and sets sound_ptr to NULL.
/// \retval true iff loading is successful. 
bool load_file(soundplayer_t* sp, const char* fname, sound_t** sound_ptr) {
    SDL_AudioSpec s;
    bool success = true;
    sound_t* sound = safe_malloc(sizeof(sound_t));

    if (!SDL_LoadWAV(fname, &s, &sound->buffer, &sound->length)) {
        fprintf(stderr, "ERROR: failed to load file \"%s\": %s\n", fname, SDL_GetError());
        success = false;
    }

    sound->stream = SDL_CreateAudioStream(&s, NULL);
    if (sound->stream == NULL) {
        fprintf(stderr, "ERROR: failed to create stream: %s\n", SDL_GetError());
        success = false;
    }
    if (!SDL_BindAudioStream(sp->device, sound->stream)) {
        fprintf(stderr, "ERROR: failed to bind stream: %s\n", SDL_GetError());
        success = false;
    }

    if (success) {
        *sound_ptr = sound;
        return true;
    }

    free(sound);
    *sound_ptr = NULL;
    return false;
}

void soundplayer_init(soundplayer_t* sp, SDL_AudioSpec spec, const char** fnames, size_t num_files) {
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        fprintf(stderr, "ERROR: failed to initialize SDL audio: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_AudioDeviceID dev_id;
    dev_id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);

    if (dev_id == 0) {
        fprintf(stderr, "ERROR: failed to open audio device: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    sp->device = dev_id;
    sp->num_sounds = num_files;
    sp->sound = safe_malloc(num_files * sizeof(sound_t*));
    for (size_t i = 0; i < num_files; ++i) {
        load_file(sp, fnames[i], &sp->sound[i]);
    }
}

void soundplayer_play(soundplayer_t sp, const int sound_id) {
    sound_t* sound = sp.sound[sound_id];
    if (sound == NULL) {
        return;
    }

    SDL_PutAudioStreamData(sound->stream, sound->buffer, sound->length);
}

void soundplayer_repeat(soundplayer_t sp, const int sound_id) {
    sound_t* sound = sp.sound[sound_id];
    if (sound == NULL) {
        return;
    }

    // add another copy of the sound if needed
    if (SDL_GetAudioStreamQueued(sound->stream) < sound->length) {
        SDL_PutAudioStreamData(sound->stream, sound->buffer, sound->length);
    }
}

void soundplayer_stop(soundplayer_t sp, const int sound_id) {
    sound_t* sound = sp.sound[sound_id];
    if (sound == NULL) {
        return;
    }

    SDL_ClearAudioStream(sound->stream);
}