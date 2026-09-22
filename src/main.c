// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "image_processing.h"

static const char *WINDOW_TITLE = "Processamento de Imagens - Projeto 1";

enum constants
{
  DEFAULT_WINDOW_WIDTH = 1024,
  DEFAULT_WINDOW_HEIGHT = 768,
  HISTOGRAM_WIDTH = 640,
  HISTOGRAM_HEIGHT = 560,
};

static MyWindow g_window = {0};
static MyWindow g_histogram_window = {0};
static MyImage g_image = {0};
static MyImage g_equalized_image = {0};
static HistogramData g_histogram = {0};
static TTF_Font *g_font = NULL;
static bool g_equalized = false;
static bool g_original_size = false;
static bool g_running = true;
static bool g_refresh = true;
static bool g_save_requested = false;
static int g_hovered_button = -1;
static int g_pressed_button = -1;
static const SDL_FRect g_buttons[2] = {
  {40, 390, 560, 52},
  {40, 456, 560, 52}
};

static MyImage *current_image(void)
{
  return g_equalized ? &g_equalized_image : &g_image;
}

static bool position_window(MyWindow *window, bool center)
{
  int top = 0, left = 0, bottom = 0, right = 0;
  SDL_GetWindowBordersSize(window->window, &top, &left, &bottom, &right);
  if (!center)
    return SDL_SetWindowPosition(window->window, left, top) &&
           SDL_SyncWindow(window->window);

  SDL_Rect display;
  int width = 0, height = 0;
  if (!SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &display) ||
      !SDL_GetWindowSize(window->window, &width, &height))
    return false;

  int x = display.x + left;
  int y = display.y + top;
  if (width + left + right <= display.w && height + top + bottom <= display.h)
  {
    x += (display.w - width - left - right) / 2;
    y += (display.h - height - top - bottom) / 2;
  }
  return SDL_SetWindowPosition(window->window, x, y) &&
         SDL_SyncWindow(window->window);
}

static bool resize_image(bool original_size)
{
  int width = original_size ? g_image.surface->w : DEFAULT_WINDOW_WIDTH;
  int height = original_size ? g_image.surface->h : DEFAULT_WINDOW_HEIGHT;
  if (!SDL_SetWindowSize(g_window.window, width, height) ||
      !SDL_SyncWindow(g_window.window) ||
      !position_window(&g_window, true))
    return false;
  g_original_size = original_size;
  return true;
}

static SDL_AppResult initialize(void)
{
  SDL_Log(">>> initialize()");

  SDL_Log("\tIniciando SDL...");
  if (!SDL_Init(SDL_INIT_VIDEO) || !TTF_Init())
  {
    SDL_Log("\t*** Erro ao iniciar SDL/SDL_ttf: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  const char *base_path = SDL_GetBasePath();
  char *font_path = NULL;
  if (!base_path ||
      SDL_asprintf(&font_path, "%sassets/fonts/DejaVuSans.ttf", base_path) < 0)
    return SDL_APP_FAILURE;
  g_font = TTF_OpenFont(font_path, 20);
  if (!g_font)
    SDL_Log("*** Erro ao carregar fonte %s: %s", font_path, SDL_GetError());
  SDL_free(font_path);
  if (!g_font)
    return SDL_APP_FAILURE;

  SDL_Log("\tCriando janela e renderizador...");
  if (!MyWindow_initialize(&g_window, WINDOW_TITLE, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0) ||
      !MyWindow_initialize(&g_histogram_window, "Histograma e controles", HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT, 0) ||
      !SDL_SetWindowParent(g_histogram_window.window, g_window.window) ||
      !position_window(&g_window, true) ||
      !position_window(&g_histogram_window, false))
  {
    SDL_Log("\t*** Erro ao configurar janelas: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_Log("<<< initialize()");
  return SDL_APP_CONTINUE;
}

static void shutdown(void)
{
  SDL_Log(">>> shutdown()");

  MyImage_destroy(&g_equalized_image);
  MyImage_destroy(&g_image);
  MyWindow_destroy(&g_histogram_window);
  MyWindow_destroy(&g_window);
  if (g_font)
  {
    TTF_CloseFont(g_font);
    g_font = NULL;
  }
  if (TTF_WasInit())
    TTF_Quit();

  SDL_Log("\tEncerrando SDL...");
  SDL_Quit();
  SDL_Log("<<< shutdown()");
}

static bool update_histogram(void)
{
  if (!calculate_histogram(current_image(), &g_histogram))
    return false;

  size_t total = 0;
  SDL_Log("Histograma (intensidade : quantidade de pixels):");
  for (int i = 0; i < 256; ++i)
  {
    SDL_Log("%3d : %u", i, g_histogram.bins[i]);
    total += g_histogram.bins[i];
  }

  size_t expected = (size_t)g_image.surface->w * g_image.surface->h;
  SDL_Log("Soma do histograma: %zu", total);
  SDL_Log("Total de pixels da imagem (%d x %d): %zu",
          g_image.surface->w, g_image.surface->h, expected);
  if (total != expected)
  {
    SDL_Log("*** Erro: contagem do histograma incorreta.");
    return false;
  }
  SDL_Log("Conferencia do histograma: OK.");
  SDL_Log("Media de intensidade: %.2f (%s)",
          g_histogram.mean, g_histogram.intensity_class);
  SDL_Log("Desvio padrao: %.2f (contraste %s)",
          g_histogram.std_dev, g_histogram.contrast_class);
  return true;
}

static bool toggle_equalization(void)
{
  if (!g_equalized_image.surface)
  {
    g_equalized_image.surface = SDL_DuplicateSurface(g_image.surface);
    if (!g_equalized_image.surface)
      return false;
    g_equalized_image.rect = g_image.rect;
    if (!equalize_histogram(g_window.renderer, &g_equalized_image, &g_histogram))
    {
      MyImage_destroy(&g_equalized_image);
      return false;
    }
  }
  g_equalized = !g_equalized;
  return update_histogram();
}

static bool draw_text(const char *text, float x, float y, SDL_Color color, bool centered)
{
  SDL_Surface *surface = TTF_RenderText_Blended(g_font, text, 0, color);
  if (!surface)
    return false;
  SDL_FRect rect = {x, y, (float)surface->w, (float)surface->h};
  if (centered)
  {
    rect.x -= rect.w / 2;
    rect.y -= rect.h / 2;
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface(g_histogram_window.renderer, surface);
  SDL_DestroySurface(surface);
  if (!texture)
    return false;
  bool success = SDL_RenderTexture(g_histogram_window.renderer, texture, NULL, &rect);
  SDL_DestroyTexture(texture);
  return success;
}

static const char *button_text(int button)
{
  if (button == 0)
    return g_equalized ? "Ver original" : "Equalizar";
  return g_original_size ? "1024x768" : "Resolução original";
}

static bool draw_button(int button)
{
  SDL_Renderer *renderer = g_histogram_window.renderer;
  SDL_Color color = {35, 99, 185, 255};
  if (g_hovered_button == button)
    color = (SDL_Color){59, 130, 230, 255};
  if (g_pressed_button == button && g_hovered_button == button)
    color = (SDL_Color){20, 62, 125, 255};
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  if (!SDL_RenderFillRect(renderer, &g_buttons[button]))
    return false;
  return draw_text(button_text(button),
                   g_buttons[button].x + g_buttons[button].w / 2,
                   g_buttons[button].y + g_buttons[button].h / 2,
                   (SDL_Color){255, 255, 255, 255}, true);
}

static bool render_histogram(void)
{
  SDL_Renderer *renderer = g_histogram_window.renderer;
  SDL_Color text_color = {28, 38, 50, 255};
  SDL_FRect graph = {64, 60, 512, 230};
  SDL_SetRenderDrawColor(renderer, 245, 246, 248, 255);
  if (!SDL_RenderClear(renderer))
    return false;
  if (!draw_text(g_equalized ? "Histograma — Equalizada" : "Histograma — Original em cinza",
                 40, 18, text_color, false))
    return false;

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  if (!SDL_RenderFillRect(renderer, &graph))
    return false;
  unsigned int maximum = 0;
  for (int i = 0; i < 256; ++i)
    if (g_histogram.bins[i] > maximum)
      maximum = g_histogram.bins[i];

  SDL_SetRenderDrawColor(renderer, 35, 99, 185, 255);
  for (int i = 0; i < 256; ++i)
  {
    float height = maximum ? (float)g_histogram.bins[i] / maximum * graph.h : 0;
    SDL_FRect bar = {graph.x + i * 2, graph.y + graph.h - height, 2, height};
    if (height > 0 && !SDL_RenderFillRect(renderer, &bar))
      return false;
  }
  SDL_SetRenderDrawColor(renderer, 90, 100, 112, 255);
  if (!SDL_RenderRect(renderer, &graph) ||
      !draw_text("0", 64, 294, text_color, false) ||
      !draw_text("255", 540, 294, text_color, false))
    return false;

  char text[160];
  SDL_snprintf(text, sizeof(text), "Média: %.2f — intensidade %s",
               g_histogram.mean, g_histogram.intensity_class);
  if (!draw_text(text, 40, 326, text_color, false))
    return false;
  SDL_snprintf(text, sizeof(text), "Desvio padrão: %.2f — contraste %s",
               g_histogram.std_dev, g_histogram.contrast_class);
  if (!draw_text(text, 40, 354, text_color, false) ||
      !draw_button(0) || !draw_button(1) ||
      !draw_text("S: salvar a imagem em output_image.png", 40, 524, text_color, false))
    return false;
  return true;
}

static bool render_image(void)
{
  SDL_SetRenderDrawColor(g_window.renderer, 128, 128, 128, 255);
  return SDL_RenderClear(g_window.renderer) &&
         SDL_RenderTexture(g_window.renderer, current_image()->texture, NULL, NULL);
}

static bool render(void)
{
  if (!render_image())
    return false;
  if (g_save_requested)
  {
    save_output_image(g_window.renderer, "output_image.png");
    g_save_requested = false;
  }
  if (!SDL_RenderPresent(g_window.renderer) || !render_histogram())
    return false;
  return SDL_RenderPresent(g_histogram_window.renderer);
}

static int button_at(float x, float y)
{
  for (int i = 0; i < 2; ++i)
    if (x >= g_buttons[i].x && x < g_buttons[i].x + g_buttons[i].w &&
        y >= g_buttons[i].y && y < g_buttons[i].y + g_buttons[i].h)
      return i;
  return -1;
}

static bool handle_event(const SDL_Event *event)
{
  SDL_WindowID main_id = SDL_GetWindowID(g_window.window);
  SDL_WindowID histogram_id = SDL_GetWindowID(g_histogram_window.window);
  switch (event->type)
  {
  case SDL_EVENT_QUIT:
    g_running = false;
    break;
  case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
    if (event->window.windowID == main_id || event->window.windowID == histogram_id)
      g_running = false;
    break;
  case SDL_EVENT_WINDOW_EXPOSED:
  case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
    g_refresh = true;
    break;
  case SDL_EVENT_MOUSE_MOTION:
    if (event->motion.windowID == histogram_id)
    {
      int hovered = button_at(event->motion.x, event->motion.y);
      if (hovered != g_hovered_button)
        g_refresh = true;
      g_hovered_button = hovered;
    }
    break;
  case SDL_EVENT_WINDOW_MOUSE_LEAVE:
    if (event->window.windowID == histogram_id)
    {
      g_hovered_button = -1;
      g_refresh = true;
    }
    break;
  case SDL_EVENT_WINDOW_FOCUS_LOST:
    if (event->window.windowID == histogram_id)
    {
      g_pressed_button = -1;
      SDL_CaptureMouse(false);
      g_refresh = true;
    }
    break;
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    if (event->button.windowID == histogram_id && event->button.button == SDL_BUTTON_LEFT)
    {
      g_hovered_button = button_at(event->button.x, event->button.y);
      g_pressed_button = g_hovered_button;
      if (g_pressed_button >= 0)
        SDL_CaptureMouse(true);
      g_refresh = true;
    }
    break;
  case SDL_EVENT_MOUSE_BUTTON_UP:
    if (event->button.button == SDL_BUTTON_LEFT)
    {
      int pressed = g_pressed_button;
      g_pressed_button = -1;
      SDL_CaptureMouse(false);
      g_hovered_button = event->button.windowID == histogram_id ?
                         button_at(event->button.x, event->button.y) : -1;
      g_refresh = true;
      if (pressed >= 0 && pressed == g_hovered_button)
      {
        if (pressed == 0 && !toggle_equalization())
          return false;
        if (pressed == 1 && !resize_image(!g_original_size))
          return false;
      }
    }
    break;
  case SDL_EVENT_KEY_DOWN:
    if ((event->key.windowID == main_id || event->key.windowID == histogram_id) &&
        event->key.key == SDLK_S && !event->key.repeat)
    {
      g_save_requested = true;
      g_refresh = true;
    }
    break;
  }
  return true;
}

static int loop(void)
{
  SDL_Log(">>> loop()");

  // Para melhorar o uso da CPU (e consumo de energia), só atualizaremos o
  // conteúdo da janela se realmente for necessário.
  while (g_running)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
      if (!handle_event(&event))
      {
        SDL_Log("*** Erro ao processar evento: %s", SDL_GetError());
        return 1;
      }

    if (g_running && g_refresh)
    {
      if (!render())
      {
        SDL_Log("*** Erro ao desenhar: %s", SDL_GetError());
        return 1;
      }
      g_refresh = false;
    }
    SDL_Delay(10);
  }

  SDL_Log("<<< loop()");
  return 0;
}

int main(int argc, char *argv[])
{
  atexit(shutdown);
  if (argc != 2)
  {
    SDL_Log("*** Uso: ./programa caminho_da_imagem");
    return 1;
  }
  if (initialize() == SDL_APP_FAILURE)
    return 1;

  load_rgba32(argv[1], g_window.renderer, &g_image);
  if (!g_image.surface || !g_image.texture)
  {
    SDL_Log("*** Erro: não foi possível carregar '%s'.", argv[1]);
    return 1;
  }
  if (!convert_to_grayscale(g_window.renderer, &g_image) || !update_histogram())
    return 1;
  return loop();
}
