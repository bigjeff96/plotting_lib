#include <SDL2/SDL.h>

void render_filled_circle(SDL_Renderer* renderer, SDL_Point pixel, int radius);
void render_filled_circle(SDL_Renderer* renderer, int x, int y, int radius);
void render_thick_line(SDL_Renderer *renderer, SDL_Point begin_pixel, SDL_Point end_pixel, float width);
int aaFilledEllipseRGBA(SDL_Renderer *renderer, float cx, float cy, float rx, float ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
