// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

#include "image_processing.h"
#include <math.h>
#include <string.h>
#include <SDL3_image/SDL_image.h>

bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags)
{
  SDL_Log("\tMyWindow_initialize(%s, %d, %d)", title, width, height);

  if (!window)
  {
    SDL_Log("\t\t*** Erro: Janela/renderizador inválidos (window == NULL).");
    return false;
  }

  return SDL_CreateWindowAndRenderer(title, width, height, window_flags, &window->window, &window->renderer);
}

void MyWindow_destroy(MyWindow *window)
{
  SDL_Log(">>> MyWindow_destroy()");

  if (!window)
  {
    SDL_Log("\t*** Erro: Janela/renderizador inválidos (window == NULL).");
    SDL_Log("<<< MyWindow_destroy()");
    return;
  }

  SDL_Log("\tDestruindo MyWindow->renderer...");
  SDL_DestroyRenderer(window->renderer);
  window->renderer = NULL;

  SDL_Log("\tDestruindo MyWindow->window...");
  SDL_DestroyWindow(window->window);
  window->window = NULL;

  SDL_Log("<<< MyWindow_destroy()");
}

void MyImage_destroy(MyImage *image)
{
  SDL_Log(">>> MyImage_destroy()");

  if (!image)
  {
    SDL_Log("\t*** Erro: Imagem inválida (image == NULL).");
    SDL_Log("<<< MyImage_destroy()");
    return;
  }

  if (image->texture)
  {
    SDL_Log("\tDestruindo MyImage->texture...");
    SDL_DestroyTexture(image->texture);
    image->texture = NULL;
  }

  if (image->surface)
  {
    SDL_Log("\tDestruindo MyImage->surface...");
    SDL_DestroySurface(image->surface);
    image->surface = NULL;
  }

  SDL_Log("\tRedefinindo MyImage->rect...");
  image->rect.x = image->rect.y = image->rect.w = image->rect.h = 0.0f;

  SDL_Log("<<< MyImage_destroy()");
}

void load_rgba32(const char *filename, SDL_Renderer *renderer, MyImage *output_image)
{
  SDL_Log(">>> load_rgba32(\"%s\")", filename);

  if (!filename)
  {
    SDL_Log("\t*** Erro: Nome do arquivo inválido (filename == NULL).");
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return;
  }

  if (!renderer)
  {
    SDL_Log("\t*** Erro: Renderer inválido (renderer == NULL).");
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return;
  }

  if (!output_image)
  {
    SDL_Log("\t*** Erro: Imagem de saída inválida (output_image == NULL).");
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return;
  }

  MyImage_destroy(output_image);

  SDL_Log("\tCarregando imagem \"%s\" em uma superfície...", filename);
  SDL_Surface *surface = IMG_Load(filename);
  if (!surface)
  {
    SDL_Log("\t*** Erro ao carregar a imagem: %s", SDL_GetError());
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return;
  }

  SDL_Log("\tConvertendo superfície para formato RGBA32...");
  output_image->surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
  SDL_DestroySurface(surface);
  if (!output_image->surface)
  {
    SDL_Log("\t*** Erro ao converter superfície para formato RGBA32: %s", SDL_GetError());
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return;
  }

  SDL_Log("\tCriando textura a partir da superfície...");
  output_image->texture = SDL_CreateTextureFromSurface(renderer, output_image->surface);
  if (!output_image->texture)
  {
    SDL_Log("\t*** Erro ao criar textura: %s", SDL_GetError());
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return;
  }

  SDL_Log("\tObtendo dimensões da textura...");
  SDL_GetTextureSize(output_image->texture, &output_image->rect.w, &output_image->rect.h);

  SDL_Log("<<< load_rgba32(\"%s\")", filename);
}

bool convert_to_grayscale(SDL_Renderer *renderer, MyImage *image)
{
  if (!renderer || !image || !image->surface)
  {
    SDL_Log("*** Erro: imagem ou renderizador invalido.");
    return false;
  }

  const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->surface->format);
  if (!format || !SDL_LockSurface(image->surface))
  {
    SDL_Log("*** Erro ao acessar os pixels: %s", SDL_GetError());
    return false;
  }

  const size_t pixelCount = (size_t)image->surface->w * image->surface->h;
  Uint32 *pixels = (Uint32 *)image->surface->pixels;
  Uint8 r = 0, g = 0, b = 0, a = 0;
  bool isGrayscale = true;

  for (size_t i = 0; i < pixelCount; ++i)
  {
    SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);

    if (r != g || g != b)
    {
      isGrayscale = false;
      Uint8 gray = (Uint8)round(0.2125 * r + 0.7154 * g + 0.0721 * b);
      pixels[i] = SDL_MapRGBA(format, NULL, gray, gray, gray, a);
    }
  }

  SDL_UnlockSurface(image->surface);

  if (isGrayscale)
  {
    SDL_Log("Imagem de entrada: em escala de cinza.");
    return true;
  }

  SDL_Log("Imagem de entrada: colorida. Convertendo para escala de cinza.");

  SDL_DestroyTexture(image->texture);
  image->texture = SDL_CreateTextureFromSurface(renderer, image->surface);
  if (!image->texture)
  {
    SDL_Log("*** Erro ao criar textura: %s", SDL_GetError());
    return false;
  }

  return true;
}

bool calculate_histogram(MyImage *image, HistogramData *hist) {
    if (!image || !image->surface || !hist) {
        SDL_Log("*** Erro: imagem ou histograma invalido.");
        return false;
    }

    memset(hist->bins, 0, sizeof(hist->bins));
    hist->mean = 0.0;
    hist->std_dev = 0.0;

    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->surface->format);
    if (!format || !SDL_LockSurface(image->surface)) {
        SDL_Log("*** Erro ao acessar pixels do histograma: %s", SDL_GetError());
        return false;
    }
    const size_t pixelCount = (size_t)image->surface->w * image->surface->h;
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
    return true;
}


bool equalize_histogram(SDL_Renderer *renderer, MyImage *image, HistogramData *hist)
{
    if (!renderer || !image || !image->surface || !hist) return false;

    const size_t pixelCount = (size_t)image->surface->w * image->surface->h;
    if (!pixelCount) return false;
    double probability[256];
    double cdf[256];
    for (int i = 0; i < 256; ++i) {
        probability[i] = (double)hist->bins[i] / pixelCount;
        cdf[i] = probability[i] + (i ? cdf[i - 1] : 0);
    }

    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->surface->format);
    if (!format || !SDL_LockSurface(image->surface)) return false;
    Uint32 *pixels = (Uint32 *)image->surface->pixels;
    Uint8 r, g, b, a;
    for (size_t i = 0; i < pixelCount; ++i) {
        SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);
        Uint8 new_val = (Uint8)round(cdf[r] * 255.0);
        pixels[i] = SDL_MapRGBA(format, NULL, new_val, new_val, new_val, a);
    }
    SDL_UnlockSurface(image->surface);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image->surface);
    if (!texture) return false;
    SDL_DestroyTexture(image->texture);
    image->texture = texture;
    return true;
}

bool save_output_image(SDL_Renderer *renderer, const char *filename)
{
    if (!renderer || !filename) return false;
    SDL_PathInfo info;
    bool existed = SDL_GetPathInfo(filename, &info) && info.type != SDL_PATHTYPE_NONE;
    SDL_Surface *surface = SDL_RenderReadPixels(renderer, NULL);
    if (!surface) {
        SDL_Log("*** Erro ao capturar imagem: %s", SDL_GetError());
        return false;
    }
    bool success = IMG_SavePNG(surface, filename);
    if (success)
        SDL_Log("Arquivo %s %s.", filename, existed ? "sobrescrito" : "criado");
    else
        SDL_Log("*** Erro ao salvar %s: %s", filename, SDL_GetError());
    SDL_DestroySurface(surface);
    return success;
}
