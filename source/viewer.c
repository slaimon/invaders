#include "../include/viewer.h"
#include <SDL3/SDL_render.h>
#include <stdio.h>

void viewer_init(viewer_t* viewer, const char* title, 
                 size_t width, size_t height, size_t scale, 
                 int pixelformat) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "ERROR: failed to initialize SDL video\n");
        exit(EXIT_FAILURE);
    }

    viewer->width = width;
    viewer->height = height;
    viewer->scale = scale;

    // create a window with scaled dimensions
    viewer->window = SDL_CreateWindow(title, width*scale, height*scale, 0);
    
    // set up the renderer with vsync on
    viewer->renderer = SDL_CreateRenderer(viewer->window, NULL);
    SDL_SetRenderVSync(viewer->renderer, 1);
    SDL_SetRenderDrawColor(viewer->renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    // create a texture for streaming access
    viewer->texture = SDL_CreateTexture(viewer->renderer, pixelformat,
                          SDL_TEXTUREACCESS_STREAMING, width, height);
}

void viewer_update(viewer_t* viewer) {
    SDL_RenderTexture(viewer->renderer, viewer->texture, NULL, NULL);
    SDL_RenderPresent(viewer->renderer);
}

void viewer_setFrame(viewer_t* viewer, uint8_t* pixelmatrix) {
    void* pixeldata;
    int pitch;
    // Lock the texture and upload the image to the GPU.
    SDL_LockTexture(viewer->texture, NULL, &pixeldata, &pitch);
    memcpy(pixeldata, pixelmatrix, pitch * viewer->height);
    SDL_UnlockTexture(viewer->texture);
}

void viewer_destroy(viewer_t* viewer) {
    SDL_DestroyTexture(viewer->texture);
    SDL_DestroyRenderer(viewer->renderer);
    SDL_DestroyWindow(viewer->window);
    SDL_Quit();
}