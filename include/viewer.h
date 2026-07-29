#include <SDL3/SDL.h>
#include <stdint.h>

// Some wrapper functions for a hardware-accelerated SDL window, optimized for texture streaming.
//
// Adapted from https://riv.dev/emulating-a-computer-part-3/

typedef struct viewer {
    uint32_t width;
    uint32_t height;
    uint32_t scale;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
} viewer_t;

// Initialize a viewer.
// Prepares a window of size width*scale x height*scale
// pixelformat is one of SDL's PIXELFORMAT enums.
// Returns false in case of error.
bool viewer_init(viewer_t* viewer, const char* title,
                 uint32_t width, uint32_t height, uint32_t scale,
                 int pixelformat);

// Render the current frame of the viewer
void viewer_update(const viewer_t* viewer);

// Set the viewer's frame to the given pixelmatrix. The matrix must conform to the viewer's pixel
// format (e.g. SDL_PIXELFORMAT_RGB24)
void viewer_setFrame(const viewer_t* viewer, const uint8_t* pixelmatrix);

// Dispose of the viewer's resources.
void viewer_destroy(viewer_t viewer);