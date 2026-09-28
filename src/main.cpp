#include "draw.h"

#include <SDL.h>
#include <cstdio>

namespace {

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Texture *texture = nullptr;
SDL_Surface *surface = nullptr;

bool init()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::printf("SDL init error: %s\n", SDL_GetError());
    return false;
  }

  window = SDL_CreateWindow(
    "Lab 2, variant 4 | LMB: rotation point, Q/E: rotate, +/- or wheel: field zoom",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  surface = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_PIXELFORMAT_ARGB8888);

  if (window == nullptr || renderer == nullptr || surface == nullptr) {
    std::printf("SDL object creation error: %s\n", SDL_GetError());
    return false;
  }

  texture = SDL_CreateTexture(renderer,
                              SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STREAMING,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT);
  return texture != nullptr;
}

void close_program()
{
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

}

int main(int, char **)
{
  if (!init()) {
    close_program();
    return 1;
  }

  reset_scene();
  bool quit = false;

  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT) {
        quit = true;
      } else if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
          quit = true;
        } else {
          handle_key(event.key.keysym.scancode);
        }
      } else if (event.type == SDL_MOUSEBUTTONDOWN &&
                 event.button.button == SDL_BUTTON_LEFT) {
        set_rotation_center(event.button.x, event.button.y);
      } else if (event.type == SDL_MOUSEMOTION &&
                 (event.motion.state & SDL_BUTTON_RMASK) != 0) {
        handle_mouse_drag(event.motion.xrel, event.motion.yrel);
      } else if (event.type == SDL_MOUSEWHEEL) {
        handle_mouse_wheel(event.wheel.y);
      }
    }

    draw(surface);
    SDL_UpdateTexture(texture, nullptr, surface->pixels, surface->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  close_program();
  return 0;
}
