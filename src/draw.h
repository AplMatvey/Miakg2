#pragma once

#include <SDL.h>

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;

void draw(SDL_Surface *surface);
void handle_key(SDL_Scancode key);
void handle_mouse_drag(int dx, int dy);
void handle_mouse_wheel(int direction);
void set_rotation_center(int x, int y);
void reset_scene();
