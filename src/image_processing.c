#include <math.h> 
#include <string.h>
#include <SDL3_image/SDL_image.h>


void calculate_histogram(MyImage *image, HistogramData *hist) {
    if (!image || !image->surface || !hist) return;

    memset(hist->bins, 0, sizeof(hist->bins));
    hist->mean = 0.0;
    hist->std_dev = 0.0;

    SDL_LockSurface(image->surface);
    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->surface->format);
    const size_t pixelCount = image->surface->w * image->surface->h;
    Uint32 *pixels = (Uint32 *)image->surface->pixels;
    
    Uint8 r, g, b, a;
    double sum = 0.0;

    
    for (size_t i = 0; i < pixelCount; ++i) {
        SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);
        hist->bins[r]++; // Como está em cinza, R=G=B
        sum += r;
    }
    
    
    hist->mean = sum / pixelCount;
    if (hist->mean < 85) strcpy(hist->intensity_class, "escura");
    else if (hist->mean > 170) strcpy(hist->intensity_class, "clara");
    else strcpy(hist->intensity_class, "média");

    
    double variance_sum = 0.0;
    for (size_t i = 0; i < pixelCount; ++i) {
        SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);
        variance_sum += pow(r - hist->mean, 2);
    }
    hist->std_dev = sqrt(variance_sum / pixelCount);

    if (hist->std_dev < 40) strcpy(hist->contrast_class, "baixo");
    else if (hist->std_dev > 80) strcpy(hist->contrast_class, "alto");
    else strcpy(hist->contrast_class, "médio");

    SDL_UnlockSurface(image->surface);
}


void equalize_histogram(SDL_Renderer *renderer, MyImage *image, HistogramData *hist) {
    if (!renderer || !image || !image->surface || !hist) return;

    const size_t pixelCount = image->surface->w * image->surface->h;
    
    
    float probability[256];
    float cdf[256];
    
    for (int i = 0; i < 256; i++) {
        probability[i] = (float)hist->bins[i] / pixelCount;
        if (i == 0) cdf[i] = probability[i];
        else cdf[i] = cdf[i - 1] + probability[i];
    }

    SDL_LockSurface(image->surface);
    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->surface->format);
    Uint32 *pixels = (Uint32 *)image->surface->pixels;
    Uint8 r, g, b, a;

    
    for (size_t i = 0; i < pixelCount; ++i) {
        SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);
        Uint8 new_val = (Uint8)(round(cdf[r] * 255.0f));
        pixels[i] = SDL_MapRGBA(format, NULL, new_val, new_val, new_val, a);
    }
    SDL_UnlockSurface(image->surface);

    
    SDL_DestroyTexture(image->texture);
    image->texture = SDL_CreateTextureFromSurface(renderer, image->surface);
}


void save_output_image(SDL_Renderer *renderer, const char *filename) {
    if (!renderer) return;

    
    SDL_Surface *surface = SDL_RenderReadPixels(renderer, NULL);
    if (!surface) {
        SDL_Log("*** Erro ao ler os pixels para salvar: %s", SDL_GetError());
        return;
    }

    if (IMG_SavePNG(surface, filename) == 0) {
        SDL_Log("Sucesso: Arquivo %s criado/sobrescrito com sucesso.", filename);
    } else {
        SDL_Log("*** Erro ao salvar o arquivo %s: %s", filename, SDL_GetError());
    }

    SDL_DestroySurface(surface);
}