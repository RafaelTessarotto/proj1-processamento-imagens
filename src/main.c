#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "image_processing.h"

static const char *WINDOW_TITLE = "Processamento de Imagens - Projeto 1";
static MyWindow g_window = { .window = NULL, .renderer = NULL };
static MyImage g_image = { .surface = NULL, .texture = NULL, .rect = { 0, 0, 0, 0 } };

static SDL_AppResult initialize(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
    if (!MyWindow_initialize(&g_window, WINDOW_TITLE, 1024, 768, 0)) return SDL_APP_FAILURE;
    return SDL_APP_CONTINUE;
}

static void shutdown(void) {
    MyImage_destroy(&g_image);
    MyWindow_destroy(&g_window);
    SDL_Quit();
}

static void render(void) {
    SDL_SetRenderDrawColor(g_window.renderer, 128, 128, 128, 255);
    SDL_RenderClear(g_window.renderer);
    SDL_RenderTexture(g_window.renderer, g_image.texture, &g_image.rect, &g_image.rect);
    SDL_RenderPresent(g_window.renderer);
}

static void loop(void) {
    bool mustRefresh = false;
    render();

    SDL_Event event;
    bool isRunning = true;
    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_1 && !event.key.repeat) {
                    convert_to_grayscale(g_window.renderer, &g_image);
                    mustRefresh = true;
                }
            }
        }
        if (mustRefresh) {
            render();
            mustRefresh = false;
        }
    }
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

    if (!g_image.surface) {
        SDL_Log("*** Erro: Não foi possível carregar '%s'.", image_path);
        return 1;
    }

    loop();
    return 0;
}