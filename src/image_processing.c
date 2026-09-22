#include "image_processing.h"
#include <SDL3_image/SDL_image.h>

bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags) {
    if (!window) return false;
    return SDL_CreateWindowAndRenderer(title, width, height, window_flags, &window->window, &window->renderer);
}

void MyWindow_destroy(MyWindow *window) {
    if (!window) return;
    if (window->renderer) SDL_DestroyRenderer(window->renderer);
    if (window->window) SDL_DestroyWindow(window->window);
    window->renderer = NULL;
    window->window = NULL;
}

void MyImage_destroy(MyImage *image) {
    if (!image) return;
    if (image->texture) SDL_DestroyTexture(image->texture);
    if (image->surface) SDL_DestroySurface(image->surface);
    image->texture = NULL;
    image->surface = NULL;
    image->rect.x = image->rect.y = image->rect.w = image->rect.h = 0.0f;
}

void load_rgba32(const char *filename, SDL_Renderer *renderer, MyImage *output_image) {
    if (!filename || !renderer || !output_image) return;
    MyImage_destroy(output_image);

    SDL_Surface *surface = IMG_Load(filename);
    if (!surface) return;

    output_image->surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);
    
    if (!output_image->surface) return;

    output_image->texture = SDL_CreateTextureFromSurface(renderer, output_image->surface);
    if (output_image->texture) {
        SDL_GetTextureSize(output_image->texture, &output_image->rect.w, &output_image->rect.h);
    }
}

void convert_to_grayscale(SDL_Renderer *renderer, MyImage *image) {
    if (!renderer || !image || !image->surface) return;

    SDL_LockSurface(image->surface);
    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->surface->format);
    const size_t pixelCount = image->surface->w * image->surface->h;
    Uint32 *pixels = (Uint32 *)image->surface->pixels;
    
    Uint8 r, g, b, a;
    for (size_t i = 0; i < pixelCount; ++i) {
        SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);
        
        Uint8 y = (Uint8)(0.2125 * r + 0.7154 * g + 0.0721 * b);
        
        pixels[i] = SDL_MapRGBA(format, NULL, y, y, y, a);
    }
    SDL_UnlockSurface(image->surface);

    SDL_DestroyTexture(image->texture);
    image->texture = SDL_CreateTextureFromSurface(renderer, image->surface);
}