#include <stdlib.h>
#include <SDL2/SDL.h>

// Some wrapper functions for a hardware-accelerated SDL window, optimized for texture streaming.
//
// Adapted from https://riv.dev/emulating-a-computer-part-3/

typedef struct viewer {
    size_t width;
    size_t height;
    size_t scale;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
} viewer_t;

// Initialize a viewer.
// Prepares a window of size width*scale x height*scale
// pixelformat is one of SDL's PIXELFORMAT enums.
void viewer_init(viewer_t* viewer, const char* title,
                 size_t width, size_t height, size_t scale,
                 int pixelformat);

// Render the current frame of the viewer
void viewer_update(viewer_t* viewer);

// Set the viewer's frame to the given pixelmatrix. The matrix must conform to the viewer's pixel
// format (e.g. SDL_PIXELFORMAT_RGB24)
void viewer_setFrame(viewer_t* viewer, uint8_t* pixelmatrix);

// destroy a viewer
void viewer_destroy(viewer_t* viewer);