#include <SDL3/SDL.h>
#include <stdbool.h>
#include "viewer.h"

bool viewer_init(viewer_t* viewer, const char* title, 
                 uint32_t width, uint32_t height, uint32_t scale, 
                 int pixelformat)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL video: %s\n", SDL_GetError());
        return false;
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

    return true;
}

void viewer_update(const viewer_t* viewer) {
    SDL_RenderTexture(viewer->renderer, viewer->texture, NULL, NULL);
    SDL_RenderPresent(viewer->renderer);
}

void viewer_setFrame(const viewer_t* viewer, const uint8_t* pixelmatrix) {
    void* pixeldata;
    int pitch;
    // Lock the texture and upload the image to the GPU.
    SDL_LockTexture(viewer->texture, NULL, &pixeldata, &pitch);
    memcpy(pixeldata, pixelmatrix, pitch * viewer->height);
    SDL_UnlockTexture(viewer->texture);
}

void viewer_destroy(viewer_t viewer) {
    SDL_DestroyTexture(viewer.texture);
    SDL_DestroyRenderer(viewer.renderer);
    SDL_DestroyWindow(viewer.window);
}