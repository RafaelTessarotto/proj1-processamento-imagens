// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "image_processing.h"

static const char *WINDOW_TITLE = "Processamento de Imagens - Projeto 1";

enum constants
{
  DEFAULT_WINDOW_WIDTH = 1024,
  DEFAULT_WINDOW_HEIGHT = 768,
};
static MyWindow g_window = { .window = NULL, .renderer = NULL };
static MyImage g_image = { .surface = NULL, .texture = NULL, .rect = { 0, 0, 0, 0 } };

static SDL_AppResult initialize(void)
{
  SDL_Log(">>> initialize()");

  SDL_Log("\tIniciando SDL...");
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    SDL_Log("\t*** Erro ao iniciar a SDL: %s", SDL_GetError());
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  SDL_Log("\tCriando janela e renderizador...");
  if (!MyWindow_initialize(&g_window, WINDOW_TITLE, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0))
  {
    SDL_Log("\t*** Erro ao criar a janela e/ou renderizador: %s", SDL_GetError());
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  SDL_Log("<<< initialize()");
  return SDL_APP_CONTINUE;
}

static void shutdown(void)
{
  SDL_Log(">>> shutdown()");

  MyImage_destroy(&g_image);
  MyWindow_destroy(&g_window);

  SDL_Log("\tEncerrando SDL...");
  SDL_Quit();

  SDL_Log("<<< shutdown()");
}

static void render(void)
{
  SDL_SetRenderDrawColor(g_window.renderer, 128, 128, 128, 255);
  SDL_RenderClear(g_window.renderer);

  SDL_RenderTexture(g_window.renderer, g_image.texture, &g_image.rect, &g_image.rect);

  SDL_RenderPresent(g_window.renderer);
}

static void loop(void)
{
  SDL_Log(">>> loop()");

  // Para melhorar o uso da CPU (e consumo de energia), só atualizaremos o
  // conteúdo da janela se realmente for necessário.
  bool mustRefresh = false;
  render();

  SDL_Event event;
  bool isRunning = true;
  while (isRunning)
  {
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_EVENT_QUIT:
        isRunning = false;
        break;

      case SDL_EVENT_WINDOW_EXPOSED:
        mustRefresh = true;
        break;
      }
    }

    if (mustRefresh)
    {
      render();
      mustRefresh = false;
    }

    SDL_Delay(50);
  }
  
  SDL_Log("<<< loop()");
}

int main(int argc, char *argv[]) {
    atexit(shutdown);

    if (argc < 2) {
        SDL_Log("*** Erro: Caminho da imagem ausente. Exemplo de uso: ./programa assets/imagem.png");
        return 1;
    }

    if (initialize() == SDL_APP_FAILURE) return SDL_APP_FAILURE;

    const char *image_path = argv[1];
    load_rgba32(image_path, g_window.renderer, &g_image);

    if (!g_image.surface || !g_image.texture) {
        SDL_Log("*** Erro: Não foi possível carregar '%s'.", image_path);
        return 1;
    }

    if (!convert_to_grayscale(g_window.renderer, &g_image)) {
        return 1;
    }

    HistogramData histogram = {0};
    if (!calculate_histogram(&g_image, &histogram)) {
        return 1;
    }

    size_t total = 0;
    SDL_Log("Histograma (intensidade : quantidade de pixels):");
    for (int i = 0; i < 256; ++i) {
        SDL_Log("%3d : %u", i, histogram.bins[i]);
        total += histogram.bins[i];
    }

    size_t expectedTotal = (size_t)g_image.surface->w * g_image.surface->h;
    SDL_Log("Soma do histograma: %zu", total);
    SDL_Log("Total de pixels da imagem (%d x %d): %zu",
            g_image.surface->w, g_image.surface->h, expectedTotal);
    if (total != expectedTotal) {
        SDL_Log("*** Erro: a contagem do histograma nao corresponde ao total de pixels.");
        return 1;
    }
    SDL_Log("Conferencia do histograma: OK.");

    SDL_Log("Media de intensidade: %.2f (%s)",
            histogram.mean, histogram.intensity_class);
    SDL_Log("Desvio padrao: %.2f (contraste %s)",
            histogram.std_dev, histogram.contrast_class);

    loop();
    return 0;
}
