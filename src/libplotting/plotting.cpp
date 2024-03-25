#include <libplotting/plotting.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

#include <cassert>
#include <cstdint>
#include <iostream>

int plot_func() {
    Plotter plotter;
    int err = plotter.init("test title", "x", "y");
    if (err) exit(1);
    plotter.plot();

    return 0;
}

uint32_t static determine_time_to_delay(uint32_t next_time) {
    uint32_t now;
    now = SDL_GetTicks();
    if (next_time <= now)
        return 0;
    else
        return next_time - now;
}

int Plotter::init(string_ref title, string_ref x_label, string_ref y_label, int width, int height) {
    this->title = title;
    this->x_label = x_label;
    this->y_label = y_label;
    if (width == 0 && height == 0) {
        this->width = SCREEN_WIDTH * (2.0 / 3.0);
        this->height = SCREEN_HEIGHT * (2.0 / 3.0);
    } else {
        this->width = width;
        this->height = height;
    }
    // here determine the data ranges

    return Plotter::init_sdl();
}

int Plotter::init_sdl() {
    int err = SDL_Init(SDL_INIT_EVERYTHING);
    if (err) return 1;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    window = SDL_CreateWindow("lul", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (!window) return 1;
    renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (!renderer) return 1;
    SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0, 0xFF);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!texture) return 1;

    // make a viewport in the center of the screen
    {  // center the plotter
        canvas_rect.w = width;
        canvas_rect.h = height;
        canvas_rect.x = (SCREEN_WIDTH - canvas_rect.w) / 2;
        canvas_rect.y = (SCREEN_HEIGHT - canvas_rect.h) / 2;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
    SDL_SetRenderTarget(renderer, texture);

    return 0;
}

void Plotter::plot() {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    // lets render to the viewport now
    SDL_RenderSetViewport(renderer, &canvas_rect);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_Point top_left = {0, 0};
    SDL_Point top_right = {canvas_rect.w - 1, 0};
    SDL_Point bottom_left = {0, canvas_rect.h - 1};
    SDL_Point bottom_right = {canvas_rect.w - 1, canvas_rect.h - 1};
    SDL_RenderDrawLine(renderer, top_left.x, top_left.y, top_right.x, top_right.y);
    SDL_RenderDrawLine(renderer, top_left.x, top_left.y, bottom_left.x, bottom_left.y);
    SDL_RenderDrawLine(renderer, bottom_left.x, bottom_left.y, bottom_right.x, bottom_right.y);
    SDL_RenderDrawLine(renderer, bottom_right.x, bottom_right.y, top_right.x, top_right.y);

    // draw the data

    // render to the whole screen again
    SDL_RenderSetViewport(renderer, NULL);

    // draw the text around the plot canvas (labels, title, legend and markings)

    // event loop
    SDL_Event e;
    SDL_SetRenderTarget(renderer, NULL);
    bool quit = false;
    static uint32_t next_time = SDL_GetTicks() + TICK_INTERVAL;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;

                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_q) {
                        quit = true;
                    }
                    break;
            }
        }
        SDL_Delay(determine_time_to_delay(next_time));
        // copy texture to the window
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        next_time += TICK_INTERVAL;
    }
}
