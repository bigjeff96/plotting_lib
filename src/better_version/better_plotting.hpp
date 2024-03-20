#pragma once
#include <SDL2/SDL.h>

#include <string>

struct Plotter {
    using string_ref = const std::string&;
    std::string title;
    std::string x_label;
    std::string y_label;
    int width;   // in pixels
    int height;  // in pixels

    int init(string_ref title, string_ref x_label, string_ref y_label, int width = 0,
             int height = 0);

    void plot();

   private:
    // 2256x1504 laptop screen
#ifdef DEBUG
    constexpr static int SCREEN_WIDTH = 2256;
    constexpr static int SCREEN_HEIGHT = 1504;
#else
    constexpr static int SCREEN_WIDTH = 1920;
    constexpr static int SCREEN_HEIGHT = 1080;
#endif
    constexpr static uint32_t TICK_INTERVAL = 33;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Rect canvas_rect;
    int init_sdl();
};

int plot_func();

#define TODO(msg)                                              \
    do {                                                       \
        printf("%s:%d:\nTODO: %s\n", __FILE__, __LINE__, msg); \
        exit(1);                                               \
    } while (0)
