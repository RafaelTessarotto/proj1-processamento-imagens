#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <SDL3/SDL.h>
#include <stdbool.h>

// Estruturas do professor
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} MyWindow;

typedef struct {
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_FRect rect;
} MyImage;

// Declaração das funções
bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags);
void MyWindow_destroy(MyWindow *window);
void MyImage_destroy(MyImage *image);
void load_rgba32(const char *filename, SDL_Renderer *renderer, MyImage *output_image);
void convert_to_grayscale(SDL_Renderer *renderer, MyImage *image);

#endif