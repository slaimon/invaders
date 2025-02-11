#include <stdlib.h>
#include <SDL2/SDL.h>

#include "../include/safe.h"
#include "../include/soundplayer.h"

sound_buffer_t* load_file(const char* fname) {
    sound_buffer_t* sound = safe_malloc(sizeof(sound_buffer_t));

    SDL_AudioSpec* r;
    SDL_ClearError();
    r = SDL_LoadWAV(fname, &sound->spec, &sound->buffer, &sound->length);

    if (r == NULL) {
        fprintf(stderr, "ERROR: failed to load file \"%s\": %s\n", fname, SDL_GetError());
        exit(EXIT_FAILURE);
    }

    return sound;
}

void soundplayer_init(soundplayer_t* sp, const char** fnames, const size_t num_files) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "ERROR: failed to initialize SDL audio\n");
        exit(EXIT_FAILURE);
    }

    sp->num_sounds = num_files;
    sp->sound = safe_malloc(num_files * sizeof(sound_buffer_t*));
    for (size_t i = 0; i < num_files; ++i) {
        sp->sound[i] = load_file(fnames[i]);
    }

    SDL_AudioDeviceID dev_id;
    dev_id = SDL_OpenAudioDevice(NULL, 0, &sp->sound[0]->spec, NULL, 0);

    if (dev_id == 0) {
        fprintf(stderr, "ERROR: failed to open audio device\n");
        exit(EXIT_FAILURE);
    }

    sp->device = dev_id;
}

void soundplayer_play(soundplayer_t sp, const int sound_id) {
    sound_buffer_t* sound = sp.sound[sound_id];
    SDL_QueueAudio(sp.device, sound->buffer, sound->length);
    SDL_PauseAudioDevice(sp.device, 0);
}