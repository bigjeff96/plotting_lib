#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_hints.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <cassert>

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    auto window = SDL_CreateWindow("lul", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    auto renderer = SDL_CreateRenderer(window, -1,
				       SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC |
				       SDL_RENDERER_TARGETTEXTURE);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0, 0xFF);
    auto texture =
	SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
			  SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    // make a viewport in the center of the screen
    SDL_Rect center_rect;
    center_rect.x = SCREEN_WIDTH / 6;
    center_rect.y = SCREEN_HEIGHT / 6;
    center_rect.w = 400;
    center_rect.h = 400;

    // lets render to the viewport now
    SDL_RenderSetViewport(renderer, &center_rect);


    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_Point top_left = {.x = 0, .y = 0};
    SDL_Point top_right = {.x = center_rect.w - 1, .y = 0};
    SDL_Point bottom_left = {.x = 0, .y = center_rect.h - 1};
    SDL_Point bottom_right = {.x = center_rect.w - 1, .y = center_rect.h - 1};

    SDL_RenderDrawLine(renderer, top_left.x, top_left.y, top_right.x, top_right.y);
    SDL_RenderDrawLine(renderer, top_left.x, top_left.y, bottom_left.x, bottom_left.y);
    SDL_RenderDrawLine(renderer, bottom_left.x, bottom_left.y, bottom_right.x, bottom_right.y);
    SDL_RenderDrawLine(renderer, bottom_right.x, bottom_right.y, top_right.x, top_right.y);

    // render to the whole screen again
    SDL_RenderSetViewport(renderer, NULL);

    SDL_Event e;
    SDL_SetRenderTarget(renderer, NULL);
    bool quit = false;
    while (!quit) {
	while (SDL_PollEvent(&e) != 0) {
	    switch (e.type) {
	    case SDL_QUIT:
		quit = true;
		break;

	    case SDL_KEYDOWN:
		if (e.key.keysym.sym == SDLK_q)
		    quit = true;
		break;
	    }
	}
	// copy texture to the window
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
    }
}
