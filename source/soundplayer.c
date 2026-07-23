#include <SDL3/SDL_audio.h>
#include <stdlib.h>

#include "soundplayer.h"

// Tries to load the given sound file and store it at sound_ptr.
// If the loading process fails, the function returns false and sets sound_ptr to NULL.
bool load_file(soundplayer_t* sp, const char* fname, sound_t** sound_ptr) {
    SDL_AudioSpec s;
    bool success = true;
    sound_t* sound = SDL_malloc(sizeof(sound_t));

    if (!SDL_LoadWAV(fname, &s, &sound->buffer, &sound->length)) {
        SDL_Log("Error loading file: %s", SDL_GetError());
        success = false;
    } else {
        sound->stream = SDL_CreateAudioStream(&s, NULL);
        if (sound->stream == NULL) {
            SDL_Log("Failed to create stream: %s", SDL_GetError());
            success = false;
        } else if (!SDL_BindAudioStream(sp->device, sound->stream)) {
            SDL_Log("Failed to bind stream: %s", SDL_GetError());
            success = false;
        }
    }

    if (success) {
        *sound_ptr = sound;
        return true;
    } else {
        SDL_free(sound);
        *sound_ptr = NULL;
        return false;
    }
}

bool soundplayer_init(soundplayer_t* sp, const char** fnames, size_t num_files) {
    sp->active = false;

    if (!SDL_Init(SDL_INIT_AUDIO)) {
        SDL_Log("Failed to initialize SDL audio: %s", SDL_GetError());
        return false;
    }

    SDL_AudioDeviceID dev_id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);

    if (dev_id == 0) {
        SDL_Log("Failed to open audio device: %s", SDL_GetError());
        return false;
    }

    sp->device = dev_id;
    sp->num_sounds = num_files;
    sp->sound = SDL_malloc(num_files * sizeof(sound_t*));
    for (size_t i = 0; i < num_files; ++i) {
        if (!load_file(sp, fnames[i], &sp->sound[i])) {
            SDL_Log("A sound file failed to load, so it will be muted.");
        }
    }

    sp->active = true;
    return true;
}

#define CHECK_IS_ACTIVE(sp) \
    if (!sp.active) return;

#define GET_SOUND(sound_id) \
    sp.sound[sound_id];     \
    if (sound == NULL) return;

void soundplayer_play(soundplayer_t sp, const int sound_id) {
    CHECK_IS_ACTIVE(sp);
    sound_t* sound = GET_SOUND(sound_id);

    SDL_PutAudioStreamData(sound->stream, sound->buffer, sound->length);
}

void soundplayer_repeat(soundplayer_t sp, const int sound_id) {
    CHECK_IS_ACTIVE(sp);
    sound_t* sound = GET_SOUND(sound_id);

    // add another copy of the sound if needed
    if (SDL_GetAudioStreamQueued(sound->stream) < sound->length) {
        SDL_PutAudioStreamData(sound->stream, sound->buffer, sound->length);
    }
}

void soundplayer_stop(soundplayer_t sp, const int sound_id) {
    CHECK_IS_ACTIVE(sp);
    sound_t* sound = GET_SOUND(sound_id);

    SDL_ClearAudioStream(sound->stream);
}

void soundplayer_set_gain(soundplayer_t sp, const int sound_id, const float gain) {
    CHECK_IS_ACTIVE(sp);
    sound_t* sound = GET_SOUND(sound_id);

    SDL_SetAudioStreamGain(sound->stream, gain);
}

void soundplayer_destroy(soundplayer_t sp) {
    CHECK_IS_ACTIVE(sp);
    
    for (int i = 0; i < sp.num_sounds; i++) {
        sound_t* sound = sp.sound[i];
        soundplayer_stop(sp, i);
        SDL_DestroyAudioStream(sound->stream);
        SDL_free(sound->buffer);
        SDL_free(sound);
    }
    SDL_CloseAudioDevice(sp.device);
    SDL_free(sp.sound);
}