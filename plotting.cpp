#include "plotting.h"
#include "render_shapes.h"
#include "rounding_nums.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <values.h>

#define view const auto&
#define mod auto&&
#define internal static

#define BETWEEN(a, b, c)                                                                                                    \
    ({                                                                                                                      \
        __typeof__(a) __a = (a);                                                                                            \
        __typeof__(b) __b = (b);                                                                                            \
        __typeof__(c) __c = (c);                                                                                            \
        (__a >= __b && __a <= __c) || (__a >= __c && __a <= __b);                                                           \
    })

#define HEX_COLOR(hex)                                                                                                      \
    ((hex) >> (3 * 8)) & 0xFF, ((hex) >> (2 * 8)) & 0xFF, ((hex) >> (1 * 8)) & 0xFF, ((hex) >> (0 * 8)) & 0xFF

#define SCREEN_WIDTH 1980
#define SCREEN_HEIGHT 1080

struct data_ranges {
    double min_x = 0.;
    double min_y = 0.;
    double max_x = 0.;
    double max_y = 0.;
    int min_x_pixel = {};
    int min_y_pixel = {};
    int max_x_pixel = {};
    int max_y_pixel = {};
};
enum COLOR { RED, BLUE, GREEN, LIGHT_BLUE, ORANGE, LIGHT_GREEN, MAGENTA, PURPLE, BLACK, GREY, COUNT_COLOR };

struct plot_data {
    int total_points = 0.;
    double* x_data = NULL;
    double* y_data = NULL;
    COLOR color = RED;
    char legend[255];
};

struct plot_array {
    plot_data* plots = NULL;
    int total_plots = 0;
};
internal plot_array arr_plots;

enum PLOT_TYPE { NORMAL_PLOT, SCATTER_PLOT, COUNT_PLOT_TYPE };

struct plot_state {
    PLOT_TYPE plot_type = NORMAL_PLOT;
    LEGEND_POSITION legend_position = TOP_RIGHT;
    bool can_set_x_label = false;
    bool can_set_y_label = false;
    bool can_save_graph_to_png = false;
    bool can_set_title = false;
    bool only_save_to_png = false;
    bool allready_init_sdl = false;
    bool can_show_legend = false;
    bool make_submarkings = false;
    char x_label[255];
    char y_label[255];
    char title[255];
    char image_path[255];
    int height = SCREEN_HEIGHT * 2 / 3;
    int width = SCREEN_WIDTH * 2 / 3;
};
internal plot_state state;

struct line {
    double coord = {};
    int pixel = {};
    LINE_TYPE type = {};
    COLOR color = {};
};

struct line_arr {
    int total = {};
    line* data = NULL;
};
internal line_arr lines;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;

void set_y_label(const char* name)
{
    state.can_set_y_label = true;
    snprintf(state.y_label, 255, "%s", name);
}
void set_x_label(const char* name)
{
    state.can_set_x_label = true;
    snprintf(state.x_label, 255, "%s", name);
}

void set_title(const char* name)
{
    state.can_set_title = true;
    snprintf(state.title, 255, "%s", name);
}

void set_aspect(int width, int height)
{
    state.width = width;
    state.height = height;
}

void make_submarkings() { state.make_submarkings = true; }

void show_legend() { state.can_show_legend = true; }

void set_legend_position(LEGEND_POSITION legend_position) { state.legend_position = legend_position; }
void save_to_png(const char* file_path)
{
    state.can_save_graph_to_png = true;
    snprintf(state.image_path, 255, "%s", file_path);
}

void only_png(const char* file_path)
{
    state.can_save_graph_to_png = true;
    snprintf(state.image_path, 255, "%s", file_path);
    state.only_save_to_png = true;
}

internal void make_plot(double x_range[2], double y_range[2])
{
    // by default, ranges = NULL
    // fill in the ranges if == NULL

    int total_plots = arr_plots.total_plots;
    plot_data* plots = arr_plots.plots;

    data_ranges ranges;
    { // set the data_ranges

        double min_x, min_y, max_x, max_y;
        double min_y_allowed, max_y_allowed;

        min_x = MAXDOUBLE;
        max_x = -MAXDOUBLE;
        min_y = MAXDOUBLE;
        max_y = -MAXDOUBLE;

        if (x_range != NULL && y_range != NULL) {
            ranges.min_x = x_range[0];
            ranges.max_x = x_range[1];
            ranges.min_y = y_range[0];
            ranges.max_y = y_range[1];
        }
        if (x_range == NULL && y_range == NULL) {

            for (int i = 0; i < total_plots; ++i) {
                double* x_data = arr_plots.plots[i].x_data;
                double* y_data = arr_plots.plots[i].y_data;
                int total_points = arr_plots.plots[i].total_points;

                for (int j = 0; j < total_points; ++j) {
                    if (min_x > x_data[j]) {
                        min_x = x_data[j];
                    }
                    if (min_y > y_data[j]) {
                        min_y = y_data[j];
                    }
                    if (max_x < x_data[j]) {
                        max_x = x_data[j];
                    }
                    if (max_y < y_data[j]) {
                        max_y = y_data[j];
                    }
                }
            }
            ranges.max_x = max_x;
            ranges.min_x = min_x;
            ranges.max_y = max_y;
            ranges.min_y = min_y;
        }
        if (x_range == NULL && y_range != NULL) {
            // x_range will only count things (1..total_points)
            ranges.min_x = 1.;
            ranges.min_y = y_range[0];
            ranges.max_y = y_range[1];

            double largest_total_points = 1.;
            for (int i = 0; i < total_plots; ++i) {
                if (arr_plots.plots[i].total_points > largest_total_points)
                    largest_total_points = arr_plots.plots[i].total_points;
            }
            ranges.max_x = largest_total_points;

            for (int i = 0; i < total_plots; ++i) {
                double* x_data = arr_plots.plots[i].x_data;
                int total_points = arr_plots.plots[i].total_points;
                for (int j = 0; j < total_points; ++j) {
                    x_data[j] = (double)j;
                }
            }
        }
        if (x_range != NULL && y_range == NULL) {
            // the y_range will be given by the points that have an x coord in the
            // allowed range
            min_y_allowed = MAXDOUBLE;
            max_y_allowed = -MAXDOUBLE;
            ranges.min_x = x_range[0];
            ranges.max_x = x_range[1];

            bool in_allowed_x_range;
            for (int i = 0; i < total_plots; ++i) {
                double* y_data = arr_plots.plots[i].y_data;
                double* x_data = arr_plots.plots[i].x_data;
                int total_points = arr_plots.plots[i].total_points;

                for (int j = 0; j < total_points; ++j) {

                    in_allowed_x_range = x_data[j] >= ranges.min_x && x_data[j] <= ranges.max_x;
                    if (min_y_allowed >= y_data[j] && in_allowed_x_range) {
                        min_y_allowed = y_data[j];
                    }
                    if (max_y_allowed <= y_data[j] && in_allowed_x_range) {
                        max_y_allowed = y_data[j];
                    }
                }
            }

            ranges.min_y = min_y_allowed;
            ranges.max_y = max_y_allowed;
        }
    }

    // find the step division that produces a round number step-size
    auto get_best_step_division = [](double& min_value_axis, double& max_value_axis) {
        int best_step_division;

        int magnetude_of_min = {};
        if (fabs(min_value_axis) > 0.0001)
            magnetude_of_min = (int)(floor(log10(fabs(min_value_axis))));
        int magnetude_of_max = {};
        if (fabs(max_value_axis) > 0.0001)
            magnetude_of_max = (int)(floor(log10(fabs(max_value_axis))));

        // Logic for when to round the data_ranges => Don't do it if the range-lenght drastically increases with
        // rounding (use it for both ends)
        {
            double proposed_rounded_min;
            double proposed_rounded_max;
            {
                double value_round_up = round_up(min_value_axis, GENERAL);
                double value_round_down = round_down(min_value_axis, GENERAL);
                double error_round_up = fabs(value_round_up - min_value_axis) / pow(10., magnetude_of_min);
                double error_round_down = fabs(value_round_down - min_value_axis) / pow(10., magnetude_of_min);

                if (error_round_up < 0.1 && error_round_down > error_round_up)
                    proposed_rounded_min = value_round_up;
                else
                    proposed_rounded_min = value_round_down;
            }
            {
                double value_round_up = round_up(max_value_axis, GENERAL);
                double value_round_down = round_down(max_value_axis, GENERAL);
                double error_round_up = fabs(value_round_up - max_value_axis) / pow(10., magnetude_of_min);
                double error_round_down = fabs(value_round_down - max_value_axis) / pow(10., magnetude_of_min);

                if (error_round_down < 0.1 && error_round_up > error_round_down)
                    proposed_rounded_max = value_round_down;
                else
                    proposed_rounded_max = value_round_up;
            }

            // logic to accept the  rounding of ranges
            double original_data_lenght = max_value_axis - min_value_axis;

            double proposed_min_values[2] = { min_value_axis, proposed_rounded_min };
            double proposed_max_values[2] = { max_value_axis, proposed_rounded_max };

            double new_data_lenghts[2][2] = {};

            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    if (i == 0 && j == 0)
                        continue;
                    new_data_lenghts[i][j] = proposed_max_values[i] - proposed_min_values[j];
                }
            }

            if (new_data_lenghts[1][1] <= 1.10 * original_data_lenght) {
                min_value_axis = proposed_rounded_min;
                max_value_axis = proposed_rounded_max;
            } else {
                if (new_data_lenghts[1][0] < new_data_lenghts[0][1] && new_data_lenghts[1][0] <= 1.10 * original_data_lenght)
                    max_value_axis = proposed_rounded_max;
                else {
                    if (new_data_lenghts[0][1] <= 1.10 * original_data_lenght)
                        min_value_axis = proposed_rounded_min;
                }
            }
        }

        const int total_divisions = 8;
        int possible_step_divisions[total_divisions] = { 3, 4, 5, 6, 7, 8, 9, 10 };
        double correction_for_step_division[total_divisions] = {};

        double data_range;

        if (magnetude_of_max - magnetude_of_min >= 3) {
            data_range = max_value_axis;
        } else {
            data_range = max_value_axis - min_value_axis;
        }

        for (int i = 0; i < total_divisions; ++i) {
            double step_size_non_rounded = (data_range) / (double)possible_step_divisions[i];
            double step_size_rounded_up = round_up(step_size_non_rounded, BASE);
            correction_for_step_division[i]
                = (fabs(step_size_rounded_up - step_size_non_rounded)) * (double)possible_step_divisions[i];
        }

        int index_smallest_correction = 0;
        for (int i = 1; i < total_divisions; ++i) {
            if (correction_for_step_division[index_smallest_correction] > correction_for_step_division[i])
                index_smallest_correction = i;
        }
        // either I increase the max value or decrease the min value, increase the
        // max
        enum SHIFT_TYPE { SHIFT_UP, SHIFT_DOWN };
        SHIFT_TYPE shift = {};
        if (fabs(max_value_axis) >= fabs(min_value_axis)) {
            max_value_axis = max_value_axis + correction_for_step_division[index_smallest_correction];
            shift = SHIFT_UP;
        } else {
            min_value_axis = min_value_axis - correction_for_step_division[index_smallest_correction];
            shift = SHIFT_DOWN;
        }

        // shift the data range down or up to guaranty round numbers will appear on the axis

        // 1. determine the step type
        if (magnetude_of_max - magnetude_of_min >= 3) {
            data_range = max_value_axis;
        } else {
            data_range = max_value_axis - min_value_axis;
        }
        best_step_division = possible_step_divisions[index_smallest_correction];
        double step_size = data_range / (double)best_step_division;
        double magnetude_of_step_size = floor(log10(step_size));
        double step_size_mantessa = step_size / pow(10., magnetude_of_step_size);
        double possible_step_types[4] = { 1., 2., 2.5, 5. };
        int index_step_type = {};

        for (int i = 0; i < 4; ++i) {
            if (fabs(step_size_mantessa - possible_step_types[i]) < 0.001) {
                index_step_type = i;
                break;
            }
        }
        int step_type;
        if (possible_step_types[index_step_type] == 2.5)
            step_type = 25;
        else
            step_type = (int)possible_step_types[index_step_type];

        if (step_type == 25)
            magnetude_of_step_size -= 1; // fair comparison for the last digit

        // 2. produce the number to compare
        int number_to_compare;
        {
            int rounded_up = ceil(fabs(min_value_axis) * pow(10., -magnetude_of_step_size));
            int rounded_down = (int)(fabs(min_value_axis) * pow(10., -magnetude_of_step_size));
            if (fabs((double)rounded_up - (double)rounded_down) / (double)rounded_up
                >= 0.10) // dog shit logic here but don't know how to deal with floats
                number_to_compare = rounded_down;
            else
                number_to_compare = rounded_up;
        }

        // 3. determine the correction if needed
        double correction_term = 0.;

        if (step_type % 2 == 0) { // for step type of 2
            // min_value_axis needs to be pair
            if (number_to_compare % 2 == 1) {
                number_to_compare--;
                correction_term = fabs((double)(number_to_compare)*pow(10., magnetude_of_step_size) - fabs(min_value_axis));
            }
        } else if (step_type % 5 == 0) {

            if (number_to_compare % 5 != 0) {
                if (number_to_compare % 10 < 5)
                    number_to_compare = number_to_compare - (number_to_compare % 10);
                else
                    number_to_compare = number_to_compare - (number_to_compare % 10) + 5;
                correction_term = fabs((double)(number_to_compare)*pow(10., magnetude_of_step_size) - fabs(min_value_axis));
            }
        }

        // shift the min and max value by the correction term
        if (shift == SHIFT_UP) {
            min_value_axis = min_value_axis - correction_term;
            max_value_axis = max_value_axis - correction_term;
        } else {
            min_value_axis = min_value_axis + correction_term;
            max_value_axis = max_value_axis + correction_term;
        }

        return best_step_division;
    };

    // change the ranges if perp lines are outside of the data ranges
    if (lines.total > 0) {
        for (int i = 0; i < lines.total; ++i) {
            switch (lines.data[i].type) {
            case HLINE: {
                if (!BETWEEN(lines.data[i].coord, ranges.min_y, ranges.max_y)) {
                    if (lines.data[i].coord < ranges.min_y)
                        ranges.min_y = lines.data[i].coord;
                    else
                        ranges.max_y = lines.data[i].coord;
                }
                break;
            }
            case VLINE: {
                if (!BETWEEN(lines.data[i].coord, ranges.min_x, ranges.max_x)) {
                    if (lines.data[i].coord < ranges.min_x)
                        ranges.min_x = lines.data[i].coord;
                    else
                        ranges.max_x = lines.data[i].coord;
                }
                break;
            }
            case LINE_TYPE_COUNT:
                assert(0 && "error");
            }
        }
    }

    // These step divisions will be use for generating the numbers that will
    // appear on the axes
    int step_division_y = get_best_step_division(ranges.min_y, ranges.max_y);
    int step_division_x = get_best_step_division(ranges.min_x, ranges.max_x);

    auto get_best_subdivision = [](double& min_value_axis, double& max_value_axis, int step_division) {
        int best_subdivision;
        const int total_subdivisions = 3;
        int possible_subdivisions[total_subdivisions] = { 2, 4, 5 };
        double correction_for_subdivision[total_subdivisions] = {};

        int magnetude_of_min = {};
        if (fabs(min_value_axis) > 0.0001)
            magnetude_of_min = (int)(floor(log10(fabs(min_value_axis))));
        int magnetude_of_max = {};
        if (fabs(max_value_axis) > 0.0001)
            magnetude_of_min = (int)(floor(log10(fabs(max_value_axis))));

        double data_range;

        if (magnetude_of_max - magnetude_of_min >= 3) {
            data_range = max_value_axis;
        } else {
            data_range = max_value_axis - min_value_axis;
        }

        double step_size = data_range / step_division;

        for (int i = 0; i < total_subdivisions; ++i) {
            double substep_size_non_rounded = step_size / (double)possible_subdivisions[i];
            double substep_size_rounded_up = round_up(substep_size_non_rounded, BASE);
            correction_for_subdivision[i] = (fabs(substep_size_rounded_up - substep_size_non_rounded));
        }

        int index_smallest_correction = 0;
        for (int i = 1; i < total_subdivisions; ++i) {
            if (correction_for_subdivision[index_smallest_correction] > correction_for_subdivision[i])
                index_smallest_correction = i;
        }

        best_subdivision = possible_subdivisions[index_smallest_correction];
        return best_subdivision;
    };

    if (lines.total > 0)
        state.make_submarkings = true;
    
    for (int i = 0; i < total_plots; ++i) {
        if (plots[i].total_points > 25) {
            state.make_submarkings = true;
            break;
        }
    }

    // best sub-division
    int substep_division_x = get_best_subdivision(ranges.min_x, ranges.max_x, step_division_x);
    int substep_division_y = get_best_subdivision(ranges.min_y, ranges.max_y, step_division_y);

    bool quit = false;
    SDL_Event e;

    // initialize SDL
    if (!state.allready_init_sdl) {
        bool success = true;
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            printf("SDL not initialized, SDL_Error: %s\n", SDL_GetError());
            success = false;
        } else {
            // anti-aliasing supposedly
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
            // making a window
            window = SDL_CreateWindow("sdl_thing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
            if (window == NULL) {
                printf("window not created, SDL_ERROR: %s\n", SDL_GetError());
                success = false;
            } else {
                // make renderer for the window
                renderer = SDL_CreateRenderer(
                    window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
                if (renderer == NULL) {
                    printf("Error in renderer creation, SDL_ERROR: %s\n", SDL_GetError());
                    success = false;
                } else {
                    // init rendering color
                    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00,
                        0xFF); // to white I think
                    texture = SDL_CreateTexture(
                        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
                    if (texture == NULL) {
                        printf("error in creating texture, SDL_ERROR: %s\n", SDL_GetError());
                    }
                    SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_NONE);
                }
                // init SDL_ttf
                if (TTF_Init() == -1) {
                    printf("SDL True type font didn't init correctly, SDL_ERROR: %s", TTF_GetError());
                    success = false;
                }

                if (state.can_save_graph_to_png) {
                    int img_flag = IMG_INIT_PNG;
                    if (!(IMG_Init(img_flag) & img_flag)) {
                        printf("SDL_Image not initialized, SDL_image_error : %s\n", IMG_GetError());
                        success = false;
                    }
                }
            }
        }
        if (!success) {
            printf("sdl failed to init \n");
            exit(1);
        }
    }
    // after init, don't need to redo an init
    state.allready_init_sdl = true;
    // lets render the graph on the texture. later we'll copy the texture to the
    // window
    SDL_SetRenderTarget(renderer, texture);

    // clear screen with pure white
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    // make a viewport in the center of the screen
    SDL_Rect center_rect;
    center_rect.x = SCREEN_WIDTH / 6;
    center_rect.y = SCREEN_HEIGHT / 6;
    center_rect.w = state.width;
    center_rect.h = state.height;

    // lets render to the viewport now
    SDL_RenderSetViewport(renderer, &center_rect);

    SDL_Point** pixels_for_each_plot;
    pixels_for_each_plot = (SDL_Point**)malloc(sizeof(SDL_Point*) * total_plots);
    // malloc for each array of pixels
    for (int i = 0; i < total_plots; ++i) {
        int total_points = arr_plots.plots[i].total_points;
        pixels_for_each_plot[i] = (SDL_Point*)malloc(sizeof(SDL_Point) * total_points);
    }

    const int PIXEL_PADDING = 15;

    auto get_color = [&](COLOR color) -> int32_t {
        switch (color) {
        case RED:
            return 0xE6194BFF;
            break;
        case BLUE:
            return 0x0082C8FF;
            break;
        case GREEN:
            return 0x3CB44BFF;
            break;
        case LIGHT_BLUE:
            return 0x46F0F0FF;
            break;
        case ORANGE:
            return 0xF58230FF;
            break;
        case LIGHT_GREEN:
            return 0xD2F53CFF;
            break;
        case MAGENTA:
            return 0xF032E6FF;
            break;
        case PURPLE:
            return 0x911EB4FF;
            break;
        case BLACK:
            return 0x000000FF;
            break;
        case GREY:
            return 0x808080FF;
            break;
        case COUNT_COLOR:
            printf("error, too many plots\n");
            exit(1);
        }
    };

    { // fix the colors of all the graphible  elements
        if (lines.total + arr_plots.total_plots > 10)
            assert(0 && "error: more than 10 graphible elements");

        // if (lines.total == 0)
        // do nothing, at the init of the plots, the colors were already fixed

        if (lines.total > 0) {
            // line colors allready fixed at init of perp_line
            // shift the plot colors by the number of perp_lines
            for (int i = 0; i < arr_plots.total_plots; ++i) {
                COLOR shifted_color = (COLOR)((int)(plots[i].color) + lines.total);
                plots[i].color = shifted_color;
            }
        }
    }

    { // lets plot some data baby!!
        ranges.min_x_pixel = 0;
        ranges.min_y_pixel = center_rect.h;
        ranges.max_x_pixel = center_rect.w;
        ranges.max_y_pixel = 0;
        { // get the pixels
            for (int i = 0; i < total_plots; ++i) {
                int total_points = arr_plots.plots[i].total_points;
                double* x_data = arr_plots.plots[i].x_data;
                double* y_data = arr_plots.plots[i].y_data;

                for (int j = 0; j < total_points; ++j) {
                    // change from data space to pixel space
                    double current_x = x_data[j];
                    double current_Y = y_data[j];
                    double x_ratio = (current_x - ranges.min_x) / (ranges.max_x - ranges.min_x);
                    int x_pixel = (int)(PIXEL_PADDING + x_ratio * (center_rect.w - 2 * PIXEL_PADDING));
                    double y_ratio = (ranges.max_y - current_Y) / (ranges.max_y - ranges.min_y);
                    int y_pixel = (int)(PIXEL_PADDING + y_ratio * (center_rect.h - 2 * PIXEL_PADDING));

                    pixels_for_each_plot[i][j].x = x_pixel;
                    pixels_for_each_plot[i][j].y = y_pixel;
                    // when x_pixel or y_pixel are negative, that means that they are
                    // outside of the accepted data ranges
                    // => they won't be seen in the viewport
                }
            }

            if (lines.total > 0) {
                for (int i = 0; i < lines.total; ++i) {
                    switch (lines.data[i].type) {
                    case HLINE: {
                        // coord is a y value
                        double current_Y = lines.data[i].coord;
                        double y_ratio = (ranges.max_y - current_Y) / (ranges.max_y - ranges.min_y);
                        int y_pixel = (int)(PIXEL_PADDING + y_ratio * (center_rect.h - 2 * PIXEL_PADDING));
                        lines.data[i].pixel = y_pixel;
                        break;
                    }
                    case VLINE: {
                        double current_x = lines.data[i].coord;
                        double x_ratio = (current_x - ranges.min_x) / (ranges.max_x - ranges.min_x);
                        int x_pixel = (int)(PIXEL_PADDING + x_ratio * (center_rect.w - 2 * PIXEL_PADDING));
                        lines.data[i].pixel = x_pixel;
                        break;
                    }
                    case LINE_TYPE_COUNT: {
                        assert(0 && "error in pixel conversion for lines");
                        break;
                    }
                    }
                }
            }
        }

        // draw red filled circle
        int radius = 3;
        if (state.plot_type == SCATTER_PLOT)
            for (int i = 0; i < total_plots; ++i) {
                int total_points = arr_plots.plots[i].total_points;
                COLOR color = arr_plots.plots[i].color;
                SDL_SetRenderDrawColor(renderer, HEX_COLOR(get_color(color)));
                for (int j = 0; j < total_points; ++j) {
                    SDL_Point pixel = pixels_for_each_plot[i][j];
                    render_filled_circle(renderer, pixel, radius);
                }
            }

        if (state.plot_type == NORMAL_PLOT) {
            auto pixel_compare_x_coord
                = [](const void* a, const void* b) { return (((SDL_Point*)a)->x - ((SDL_Point*)b)->x); };
            for (int i = 0; i < total_plots; ++i) {
                int total_points = arr_plots.plots[i].total_points;
                COLOR color = arr_plots.plots[i].color;
                qsort(pixels_for_each_plot[i], total_points, sizeof(SDL_Point), pixel_compare_x_coord);

                SDL_Point begin_pixel, end_pixel;
                // SDL_SetRenderDrawColor(renderer, HEX_COLOR(get_color(color)));
                for (int j = 0; j < total_points - 1; ++j) {
                    begin_pixel = pixels_for_each_plot[i][j];
                    end_pixel = pixels_for_each_plot[i][j + 1];
                    float width = 2;
                    // render_thick_line(renderer, begin_pixel, end_pixel, width);
                    thickLineRGBA(renderer, begin_pixel.x, begin_pixel.y, end_pixel.x, end_pixel.y, width,
                        HEX_COLOR((get_color(color))));
                }
            }
        }

        // render the hlines and vlines
        if (lines.total > 0) {
            SDL_Point begin_pixel, end_pixel;
            for (int i = 0; i < lines.total; ++i) {
                switch (lines.data[i].type) {
                case HLINE: {
                    float width = 4;
                    begin_pixel.y = end_pixel.y = lines.data[i].pixel;
                    begin_pixel.x = ranges.min_x_pixel;
                    end_pixel.x = ranges.max_x_pixel;
                    thickLineRGBA(renderer, begin_pixel.x, begin_pixel.y, end_pixel.x, end_pixel.y, width,
                        HEX_COLOR(get_color(lines.data[i].color)));
                    break;
                }
                case VLINE: {
                    float width = 4;
                    begin_pixel.x = end_pixel.x = lines.data[i].pixel;
                    begin_pixel.y = ranges.min_y_pixel;
                    end_pixel.y = ranges.max_y_pixel;
                    thickLineRGBA(renderer, begin_pixel.x, begin_pixel.y, end_pixel.x, end_pixel.y, width,
                        HEX_COLOR(get_color(lines.data[i].color)));
                    break;
                }
                case LINE_TYPE_COUNT: {
                    assert(0 && "error in rendering the lines ");
                }
                }
            }
        }
    }

    { // Lets put stuff around the data, nums and text
        // draw black frame around plot
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_Point top_left = { .x = 0, .y = 0 };
        SDL_Point top_right = { .x = center_rect.w - 1, .y = 0 };
        SDL_Point bottom_left = { .x = 0, .y = center_rect.h - 1 };
        SDL_Point bottom_right = { .x = center_rect.w - 1, .y = center_rect.h - 1 };

        SDL_RenderDrawLine(renderer, top_left.x, top_left.y, top_right.x, top_right.y);
        SDL_RenderDrawLine(renderer, top_left.x, top_left.y, bottom_left.x, bottom_left.y);
        SDL_RenderDrawLine(renderer, bottom_left.x, bottom_left.y, bottom_right.x, bottom_right.y);
        SDL_RenderDrawLine(renderer, bottom_right.x, bottom_right.y, top_right.x, top_right.y);

        // render to the whole screen again
        SDL_RenderSetViewport(renderer, NULL);

        // write text to the screen
        TTF_Font* font = TTF_OpenFont("OpenSans-Regular.ttf", 20);
        if (!font) {
            printf("error in loading font, SDL_ERROR: %s\n", SDL_GetError());
            exit(1);
        }
        // render some text
        SDL_Color text_color = { 0, 0, 0, 255 };

        struct text_data {
            int height = {};
            int width = {};
            int x = {};
            int y = {};
            SDL_Texture* texture = NULL;
        };

        auto make_text_data = [&](TTF_Font* font, const char* string) {
            SDL_Surface* text_surface = TTF_RenderText_Blended(font, string, text_color);
            SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            text_data data;
            data.height = text_surface->h;
            data.width = text_surface->w;
            data.texture = text_texture;
            SDL_FreeSurface(text_surface);
            return data;
        };

        auto render_text = [&](text_data& text) {
            SDL_Rect text_rect = { .x = text.x, .y = text.y, .w = text.width, .h = text.height };
            SDL_RenderCopy(renderer, text.texture, NULL, &text_rect);
            SDL_DestroyTexture(text.texture);
        };

        auto render_text_rotate = [&](text_data& text, double rotation_in_deg) {
            SDL_Rect text_rect = { .x = text.x, .y = text.y, .w = text.width, .h = text.height };
            SDL_RenderCopyEx(renderer, text.texture, NULL, &text_rect, rotation_in_deg, NULL, SDL_FLIP_NONE);
            SDL_DestroyTexture(text.texture);
        };

        // to take into account PIXEL_PADDING
        SDL_Rect center_rect_with_padding = { .x = center_rect.x + PIXEL_PADDING,
            .y = center_rect.y + PIXEL_PADDING,
            .w = center_rect.w - 2 * PIXEL_PADDING,
            .h = center_rect.h - 2 * PIXEL_PADDING };

        enum WHICH_AXIS { X_AXIS, Y_AXIS, COUNT_WHICH_AXIS };
        auto put_nums_on_axis = [&](double min_value_axis, double max_value_axis, int step_division, int substep_division,
                                    WHICH_AXIS AXIS) {
            double axis_nums_step = (max_value_axis - min_value_axis) / step_division;
            char axis_num_str[255];

            for (int i = 0; i < step_division + 1; ++i) {
                double num_on_axis = min_value_axis + (double)i * axis_nums_step;
                if (BETWEEN(num_on_axis, -1e-8, 1e-8))
                    num_on_axis = 0.;
                snprintf(axis_num_str, 255, "%.3g", num_on_axis);

                text_data text = make_text_data(font, axis_num_str);
                if (AXIS == Y_AXIS) {
                    int text_padding = 13;
                    text.y = center_rect_with_padding.y - text.height / 2
                        + (step_division - i) * (center_rect_with_padding.h / step_division);
                    text.x = center_rect.x - text.width - text_padding;
                }
                if (AXIS == X_AXIS) {
                    int text_padding = 20;
                    text.y = center_rect.y + center_rect.h + text.height - text_padding;
                    text.x = center_rect_with_padding.x - text.width / 2 + i * (center_rect_with_padding.w / step_division);
                }
                // write the text
                render_text(text);
            }
            // draw segments, independent of text
            if (AXIS == X_AXIS) {
                int segment_lenght = 9;
                int sub_segment_lenght = 4;
                for (int i = 0; i < step_division + 1; ++i) {
                    int current_section = center_rect_with_padding.x + i * (center_rect_with_padding.w / step_division);
                    SDL_RenderDrawLine(renderer, current_section, center_rect.y + center_rect.h, current_section,
                        center_rect.y + center_rect.h + segment_lenght);
                    if (state.make_submarkings) {
                        if (i < step_division)
                            for (int j = 0; j < substep_division - 1; ++j) {
                                int substep_size = (center_rect_with_padding.w / step_division) / substep_division;
                                SDL_RenderDrawLine(renderer, current_section + (j + 1) * substep_size,
                                    center_rect.y + center_rect.h, current_section + (j + 1) * substep_size,
                                    center_rect.y + center_rect.h + sub_segment_lenght);
                            }
                    }
                }
            }
            if (AXIS == Y_AXIS) {
                int segment_lenght = 9;
                int sub_segment_lenght = 4;
                for (int i = 0; i < step_division + 1; ++i) {
                    int current_starting_section
                        = center_rect_with_padding.y + i * (center_rect_with_padding.h / step_division);
                    SDL_RenderDrawLine(renderer, center_rect.x, current_starting_section, center_rect.x - segment_lenght,
                        current_starting_section);
                    if (state.make_submarkings) {
                        if (i < step_division)
                            for (int j = 0; j < substep_division - 1; ++j) {
                                int substep_size = (center_rect_with_padding.h / step_division) / substep_division;
                                SDL_RenderDrawLine(renderer, center_rect.x,
                                    current_starting_section + (j + 1) * substep_size, center_rect.x - sub_segment_lenght,
                                    current_starting_section + (j + 1) * substep_size);
                            }
                    }
                }
            }
        };

        put_nums_on_axis(ranges.min_x, ranges.max_x, step_division_x, substep_division_x, X_AXIS);
        put_nums_on_axis(ranges.min_y, ranges.max_y, step_division_y, substep_division_y, Y_AXIS);
        TTF_CloseFont(font);

        // writing the labels and the title
        if (state.can_set_x_label || state.can_set_y_label) { // put labels on the axes
            TTF_Font* label_font = TTF_OpenFont("OpenSans-Regular.ttf", 20);

            if (state.can_set_x_label) { // x axis label
                text_data text = make_text_data(label_font, state.x_label);
                int text_padding = 50;
                text.x = center_rect.x + center_rect.w / 2 - text.width / 2;
                text.y = center_rect.y + center_rect.h + text_padding;
                render_text(text);
            }

            if (state.can_set_y_label) { // y axis label
                text_data text = make_text_data(label_font, state.y_label);
                int text_padding = 110;
                text.x = center_rect.x - text.width / 2 - text.height / 2 - text_padding;
                text.y = center_rect.y + (center_rect.h - text.height) / 2;
                render_text_rotate(text, 270.);
            }
            TTF_CloseFont(label_font);
        }

        if (state.can_set_title) {
            TTF_Font* title_font = TTF_OpenFont("OpenSans-Regular.ttf", 35);
            text_data text = make_text_data(title_font, state.title);
            int text_padding = 90;
            text.x = center_rect.x + center_rect.w / 2 - text.width / 2;
            text.y = center_rect.y - text_padding;
            render_text(text);
            TTF_CloseFont(title_font);
        }

        if (arr_plots.total_plots > 1 || state.can_show_legend) {
            TTF_Font* legend_font = TTF_OpenFont("OpenSans-Regular.ttf", 13);
            text_data text[total_plots];
            int max_text_width = MININT;
            for (int i = 0; i < total_plots; ++i) {
                if (plots[i].legend[0] == 0) {
                    char legend[255];
                    snprintf(legend, 255, "plot %d", i + 1);
                    text[i] = make_text_data(legend_font, legend);
                } else {
                    text[i] = make_text_data(legend_font, plots[i].legend);
                }
                if (text[i].width > max_text_width)
                    max_text_width = text[i].width;
            }

            int top_padding = 10;
            int bottom_padding = 7;
            int marker_padding = 9;
            int better_marker_height = 2;
            int marker_radius = 4;

            for (int i = 0; i < total_plots; ++i) {
                switch (state.legend_position) {
                case TOP_RIGHT: {
                    text[i].x = center_rect_with_padding.x + center_rect_with_padding.w - max_text_width;
                    text[i].y = center_rect_with_padding.y - top_padding + i * text[i].height;
                    break;
                }
                case BOTTOM_RIGHT: {
                    text[i].x = center_rect_with_padding.x + center_rect_with_padding.w - max_text_width;
                    text[i].y = center_rect_with_padding.y + center_rect_with_padding.h + bottom_padding
                        - (total_plots - i) * text[i].height;
                    break;
                }
                case BOTTOM_LEFT: {
                    text[i].y = center_rect_with_padding.y + center_rect_with_padding.h + bottom_padding
                        - (total_plots - i) * text[i].height;
                    text[i].x = center_rect_with_padding.x + marker_padding;
                    break;
                }
                case TOP_LEFT: {
                    text[i].x = center_rect_with_padding.x + marker_padding;
                    text[i].y = center_rect_with_padding.y - top_padding + i * text[i].height;
                    break;
                }
                case COUNT_LEGEND_POSITION: {
                    assert(0 && "error in legend position");
                    break;
                }
                }
            }

            int legend_spacing = 5;
            SDL_Rect legend_rect = { .x = text[0].x - marker_padding - marker_radius - legend_spacing,
                .y = text[0].y,
                .w = max_text_width + 2 * marker_padding + marker_radius + legend_spacing,
                .h = total_plots * text[0].height + legend_spacing };
            // do a renderclear for the legend
            SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
            SDL_RenderFillRect(renderer, &legend_rect);
            // render a rectangle around the legend

            // test

            // render the legend
            for (int i = 0; i < total_plots; ++i) {
                render_text(text[i]);
                aacircleRGBA(renderer, text[i].x - marker_padding, text[i].y + text[i].height / 2 + better_marker_height,
                    marker_radius, HEX_COLOR(get_color(plots[i].color)));
                // filledCircleRGBA(renderer, text[i].x - marker_padding, text[i].y + text[i].height / 2 + better_marker_height,
                    // marker_radius, HEX_COLOR(get_color(plots[i].color)));
                SDL_SetRenderDrawColor(renderer, HEX_COLOR(get_color(plots[i].color)));
                render_filled_circle(renderer, text[i].x - marker_padding, text[i].y + text[i].height / 2 + better_marker_height,
                                     marker_radius);
            }
            TTF_CloseFont(legend_font);
        }
    }

    if (state.can_save_graph_to_png) { // Save the file to png
        int width, height;
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
        SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
        SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);
        IMG_SavePNG(surface, state.image_path);
        SDL_FreeSurface(surface);
    }

    if (!state.only_save_to_png) {
        // render to the window now
        SDL_SetRenderTarget(renderer, NULL);
        while (!quit) {
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
            }
            // copy texture to the window
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }
    }

    { // free the plot data, the pixel data, and the line data
        for (int i = 0; i < total_plots; ++i) {
            free(pixels_for_each_plot[i]);
            memset(arr_plots.plots[i].legend, 0, 255);
        }
        free(pixels_for_each_plot);
        free(arr_plots.plots);
        arr_plots.total_plots = 0;

        free(lines.data);
        lines.total = 0;
    }

    // the os will free the window, renderer, and the texture

    { // reset the plot state
        memset(state.x_label, 0, 255);
        memset(state.y_label, 0, 255);
        memset(state.image_path, 0, 255);
        state.plot_type = NORMAL_PLOT;
        state.can_set_y_label = false;
        state.can_set_x_label = false;
        state.can_save_graph_to_png = false;
        state.only_save_to_png = false;
        state.legend_position = TOP_RIGHT;
        state.height = SCREEN_HEIGHT * 2 / 3;
        state.width = SCREEN_WIDTH * 2 / 3;
    }
}

void plot(double x_data[], double y_data[], int total_points, double x_range[2], double y_range[2])
{
    add_plot(x_data, y_data, total_points);
    make_plot(x_range, y_range);
}

void scatter_plot(double x_data[], double y_data[], int total_points, double x_range[2], double y_range[2])
{
    state.plot_type = SCATTER_PLOT;
    add_plot(x_data, y_data, total_points);
    make_plot(x_range, y_range);
}

void draw_perp_line(double value, LINE_TYPE type)
{
    lines.total++;
    lines.data = (line*)realloc(lines.data, lines.total * sizeof(line));
    int index = lines.total - 1;
    lines.data[index].coord = value;
    lines.data[index].type = type;
    lines.data[index].color = (COLOR)index;
}

void add_plot(double x_data[], double y_data[], int total_points, const char* legend)
{
    arr_plots.total_plots++;
    int index = arr_plots.total_plots - 1;
    arr_plots.plots = (plot_data*)realloc(arr_plots.plots, sizeof(plot_data) * arr_plots.total_plots);
    arr_plots.plots[index].color = (COLOR)(index);
    arr_plots.plots[index].x_data = x_data;
    arr_plots.plots[index].y_data = y_data;
    arr_plots.plots[index].total_points = total_points;
    if (legend)
        snprintf(arr_plots.plots[index].legend, 255, "%s", legend);
    else
        memset(arr_plots.plots[index].legend, 0, 255);
}

void plot_all(double x_range[2], double y_range[2]) { make_plot(x_range, y_range); }

void scatter_plot_all(double x_range[2], double y_range[2])
{
    state.plot_type = SCATTER_PLOT;
    make_plot(x_range, y_range);
}
