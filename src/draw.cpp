#include "draw.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

constexpr double PI = 3.14159265358979323846;
constexpr int VARIANT = 4;

struct Point {
  double x;
  double y;
};

struct Matrix {
  double a[3][3];
};

Matrix transform{};
Point rotation_center{SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0};
double field_scale = 1.0;
int figure_count = 24;
int vertex_count = 4;
Matrix identity()
{
  return {{{1.0, 0.0, 0.0},
           {0.0, 1.0, 0.0},
           {0.0, 0.0, 1.0}}};
}

Matrix multiply(const Matrix &left, const Matrix &right)
{
  Matrix result{};
  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 3; ++column) {
      for (int k = 0; k < 3; ++k) {
        result.a[row][column] += left.a[row][k] * right.a[k][column];
      }
    }
  }
  return result;
}

Matrix translation(double dx, double dy)
{
  Matrix result = identity();
  result.a[0][2] = dx;
  result.a[1][2] = dy;
  return result;
}

Matrix scaling(double value)
{
  Matrix result = identity();
  result.a[0][0] = value;
  result.a[1][1] = value;
  return result;
}

Matrix rotation(double angle)
{
  Matrix result = identity();
  result.a[0][0] = std::cos(angle);
  result.a[0][1] = -std::sin(angle);
  result.a[1][0] = std::sin(angle);
  result.a[1][1] = std::cos(angle);
  return result;
}

Matrix rotate_around_point(double angle)
{
  return multiply(
    translation(rotation_center.x, rotation_center.y),
    multiply(rotation(angle), translation(-rotation_center.x, -rotation_center.y)));
}

void scale_field(double factor)
{
  const double new_scale = std::max(0.25, std::min(field_scale * factor, 4.0));
  const double actual_factor = new_scale / field_scale;
  field_scale = new_scale;

  const double center_x = SCREEN_WIDTH / 2.0;
  const double center_y = SCREEN_HEIGHT / 2.0;
  const Matrix zoom = multiply(
    translation(center_x, center_y),
    multiply(scaling(actual_factor), translation(-center_x, -center_y)));

  transform = multiply(zoom, transform);
  rotation_center.x = center_x + (rotation_center.x - center_x) * actual_factor;
  rotation_center.y = center_y + (rotation_center.y - center_y) * actual_factor;
}

Point apply_transform(const Point &point)
{
  return {
    transform.a[0][0] * point.x + transform.a[0][1] * point.y + transform.a[0][2],
    transform.a[1][0] * point.x + transform.a[1][1] * point.y + transform.a[1][2]
  };
}

void put_pixel(SDL_Surface *surface, int x, int y, Uint32 color)
{
  if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
    return;
  }

  auto *pixels = static_cast<Uint32 *>(surface->pixels);
  const int pixels_per_row = surface->pitch / static_cast<int>(sizeof(Uint32));
  pixels[y * pixels_per_row + x] = color;
}
void line_bresenham(SDL_Surface *surface, Point first, Point second, Uint32 color)
{
  int x1 = static_cast<int>(std::lround(first.x));
  int y1 = static_cast<int>(std::lround(first.y));
  const int x2 = static_cast<int>(std::lround(second.x));
  const int y2 = static_cast<int>(std::lround(second.y));

  const int dx = std::abs(x2 - x1);
  const int dy = std::abs(y2 - y1);
  const int step_x = (x1 < x2) ? 1 : -1;
  const int step_y = (y1 < y2) ? 1 : -1;
  int error = dx - dy;

  while (true) {
    put_pixel(surface, x1, y1, color);
    if (x1 == x2 && y1 == y2) {
      break;
    }

    const int doubled_error = 2 * error;
    if (doubled_error > -dy) {
      error -= dy;
      x1 += step_x;
    }
    if (doubled_error < dx) {
      error += dx;
      y1 += step_y;
    }
  }
}

void draw_grid(SDL_Surface *surface)
{
  const Uint32 grid_color = SDL_MapRGB(surface->format, 218, 224, 232);
  const Uint32 axis_color = SDL_MapRGB(surface->format, 120, 130, 145);
  const int step = std::max(10, static_cast<int>(std::lround(50.0 * field_scale)));

  for (int x = SCREEN_WIDTH / 2; x < SCREEN_WIDTH; x += step) {
    line_bresenham(surface, {static_cast<double>(x), 0.0},
                   {static_cast<double>(x), SCREEN_HEIGHT - 1.0}, grid_color);
  }
  for (int x = SCREEN_WIDTH / 2 - step; x >= 0; x -= step) {
    line_bresenham(surface, {static_cast<double>(x), 0.0},
                   {static_cast<double>(x), SCREEN_HEIGHT - 1.0}, grid_color);
  }
  for (int y = SCREEN_HEIGHT / 2; y < SCREEN_HEIGHT; y += step) {
    line_bresenham(surface, {0.0, static_cast<double>(y)},
                   {SCREEN_WIDTH - 1.0, static_cast<double>(y)}, grid_color);
  }
  for (int y = SCREEN_HEIGHT / 2 - step; y >= 0; y -= step) {
    line_bresenham(surface, {0.0, static_cast<double>(y)},
                   {SCREEN_WIDTH - 1.0, static_cast<double>(y)}, grid_color);
  }

  line_bresenham(surface, {SCREEN_WIDTH / 2.0, 0.0},
                 {SCREEN_WIDTH / 2.0, SCREEN_HEIGHT - 1.0}, axis_color);
  line_bresenham(surface, {0.0, SCREEN_HEIGHT / 2.0},
                 {SCREEN_WIDTH - 1.0, SCREEN_HEIGHT / 2.0}, axis_color);
}

void draw_rotation_center(SDL_Surface *surface)
{
  const Uint32 color = SDL_MapRGB(surface->format, 210, 45, 55);
  line_bresenham(surface, {rotation_center.x - 8.0, rotation_center.y},
                 {rotation_center.x + 8.0, rotation_center.y}, color);
  line_bresenham(surface, {rotation_center.x, rotation_center.y - 8.0},
                 {rotation_center.x, rotation_center.y + 8.0}, color);
}

std::vector<Point> make_polygon()
{
  std::vector<Point> points;
  const double radius = 220.0;

  for (int i = 0; i < vertex_count; ++i) {
    const double angle = -PI / 2.0 + 2.0 * PI * i / vertex_count;
    points.push_back({radius * std::cos(angle), radius * std::sin(angle)});
  }
  return points;
}

double nesting_coefficient()
{
  const int steps = figure_count - 1;
  const double side_angle = 2.0 * PI / vertex_count;
  const double turn = VARIANT * PI / (vertex_count * steps);
  const double tangent = std::tan(turn);
  return tangent / (std::sin(side_angle) + tangent * (1.0 - std::cos(side_angle)));
}

}

void reset_scene()
{
  transform = translation(SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0);
  rotation_center = {SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0};
  field_scale = 1.0;
  figure_count = 24;
  vertex_count = 4;
}

void handle_key(SDL_Scancode key)
{
  constexpr double move_step = 10.0;
  constexpr double angle_step = 5.0 * PI / 180.0;

  switch (key) {
  case SDL_SCANCODE_W:
  case SDL_SCANCODE_UP:
    transform = multiply(translation(0.0, -move_step), transform);
    break;
  case SDL_SCANCODE_S:
  case SDL_SCANCODE_DOWN:
    transform = multiply(translation(0.0, move_step), transform);
    break;
  case SDL_SCANCODE_A:
  case SDL_SCANCODE_LEFT:
    transform = multiply(translation(-move_step, 0.0), transform);
    break;
  case SDL_SCANCODE_D:
  case SDL_SCANCODE_RIGHT:
    transform = multiply(translation(move_step, 0.0), transform);
    break;
  case SDL_SCANCODE_Q:
    transform = multiply(rotate_around_point(-angle_step), transform);
    break;
  case SDL_SCANCODE_E:
    transform = multiply(rotate_around_point(angle_step), transform);
    break;
  case SDL_SCANCODE_EQUALS:
  case SDL_SCANCODE_KP_PLUS:
    scale_field(1.1);
    break;
  case SDL_SCANCODE_MINUS:
  case SDL_SCANCODE_KP_MINUS:
    scale_field(1.0 / 1.1);
    break;
  case SDL_SCANCODE_N:
    figure_count = std::min(figure_count + 1, 100);
    break;
  case SDL_SCANCODE_M:
    figure_count = std::max(figure_count - 1, 6);
    break;
  case SDL_SCANCODE_RIGHTBRACKET:
    vertex_count = std::min(vertex_count + 1, 12);
    break;
  case SDL_SCANCODE_LEFTBRACKET:
    vertex_count = std::max(vertex_count - 1, 3);
    break;
  case SDL_SCANCODE_R:
    reset_scene();
    break;
  default:
    break;
  }
}

void handle_mouse_drag(int dx, int dy)
{
  transform = multiply(translation(dx, dy), transform);
}

void handle_mouse_wheel(int direction)
{
  if (direction > 0) {
    scale_field(1.1);
  } else if (direction < 0) {
    scale_field(1.0 / 1.1);
  }
}

void set_rotation_center(int x, int y)
{
  rotation_center = {static_cast<double>(x), static_cast<double>(y)};
}

void draw(SDL_Surface *surface)
{
  const Uint32 background = SDL_MapRGB(surface->format, 245, 247, 250);
  const Uint32 line_color = SDL_MapRGB(surface->format, 25, 90, 160);
  SDL_FillRect(surface, nullptr, background);

  if (SDL_MUSTLOCK(surface)) {
    SDL_LockSurface(surface);
  }

  draw_grid(surface);
  std::vector<Point> polygon = make_polygon();
  const double mu = nesting_coefficient();

  for (int number = 0; number < figure_count; ++number) {
    for (int i = 0; i < vertex_count; ++i) {
      const int next = (i + 1) % vertex_count;
      line_bresenham(surface,
                     apply_transform(polygon[i]),
                     apply_transform(polygon[next]),
                     line_color);
    }

    std::vector<Point> inner(vertex_count);
    for (int i = 0; i < vertex_count; ++i) {
      const int next = (i + 1) % vertex_count;
      inner[i].x = (1.0 - mu) * polygon[i].x + mu * polygon[next].x;
      inner[i].y = (1.0 - mu) * polygon[i].y + mu * polygon[next].y;
    }
    polygon = inner;
  }

  draw_rotation_center(surface);

  if (SDL_MUSTLOCK(surface)) {
    SDL_UnlockSurface(surface);
  }
}
