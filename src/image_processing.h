#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} MyWindow;

typedef struct {
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_FRect rect;
} MyImage;

typedef struct {
    unsigned int bins[256];
    double mean;
    double std_dev;
    char intensity_class[10]; // "clara", "média", "escura"
    char contrast_class[10];  // "alto", "médio", "baixo"
} HistogramData;

bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags);
void MyWindow_destroy(MyWindow *window);
void MyImage_destroy(MyImage *image);
void load_rgba32(const char *filename, SDL_Renderer *renderer, MyImage *output_image);
void convert_to_grayscale(SDL_Renderer *renderer, MyImage *image);

void calculate_histogram(MyImage *image, HistogramData *hist);
void equalize_histogram(SDL_Renderer *renderer, MyImage *image, HistogramData *hist);
void save_output_image(SDL_Renderer *renderer, const char *filename);

#endif