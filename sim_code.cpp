#define STB_DS_IMPLEMENTATION
#include <stb/stb_ds.h>
#include <stdio.h>

#include "plotting.h"
#include <math.h>
const double GRAVITY         = -9.81f;
const double TOTAL_TIME      = 5.0f;
const double MASS            = 5.0f;
const double DISSIPATION_CST = 0.8f;
const double RADIUS          = 0.06f;
const double SPRING_CST      = -2.0f * MASS * GRAVITY / (RADIUS * RADIUS / 10000.0f);
const double VISCOSITY_SPRING =
    -2.0f * log(DISSIPATION_CST) * sqrt(MASS * SPRING_CST / (log(DISSIPATION_CST * DISSIPATION_CST) + M_PI * M_PI));
const double VISCOSITY_AIR = 0.1f;
const double TIME_STEP     = 0.05f * M_PI * sqrt(MASS / SPRING_CST) * 1.0f;

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    double *height_of_ball = NULL;
    double *speed_of_ball  = NULL;
    double *time           = NULL;

    arrpush(height_of_ball, 1.0f);
    arrpush(speed_of_ball, 0.0f);
    arrpush(time, 0.0f);

    double current_time = 0.0f;

    double new_height, new_half_height, new_speed, delta;
    while (current_time < TOTAL_TIME) {
        new_half_height = arrlast(height_of_ball) + arrlast(speed_of_ball) * TIME_STEP / 2.0f;
        delta           = new_half_height - RADIUS;
        arrlast(speed_of_ball);

        if (delta < 0.0f) {
            new_speed = arrlast(speed_of_ball) + GRAVITY * TIME_STEP - (SPRING_CST * delta * TIME_STEP / MASS) -
                        TIME_STEP * VISCOSITY_SPRING * arrlast(speed_of_ball) / MASS -
                        (arrlast(speed_of_ball) * TIME_STEP * VISCOSITY_AIR) / MASS;
        } else {
            new_speed =
                arrlast(speed_of_ball) + GRAVITY * TIME_STEP - TIME_STEP * VISCOSITY_AIR * arrlast(speed_of_ball) / MASS;
        }

        new_height = new_half_height + new_speed * TIME_STEP / 2.0f;
        arrpush(height_of_ball, new_height);
        arrpush(speed_of_ball, new_speed);
        current_time += TIME_STEP;
        arrpush(time, current_time);
    }

    int total_data_points = (int)(TOTAL_TIME / TIME_STEP);

    double y_range[2] = { -0.01, 0.01 };
    double x_range[2] = { 4., 5. };

    set_x_label("Time in seconds");
    set_y_label("Height & speed of the ball");
    set_aspect(700, 700);
    set_title("Height & speed of the ball over time");
    add_plot(time, height_of_ball, total_data_points, "height of the ball");
    add_plot(time, speed_of_ball, total_data_points, "speed of the ball");
    // set_legend_position(BOTTOM_RIGHT);
    show_legend();
    // draw_perp_line(3.5, VLINE);
    // draw_perp_line(0., HLINE);
    // only_png("PNG/test_test.png");
    // set_aspect(500, 400);
    plot_all();

    // set_x_label("Time in seconds");
    // set_y_label("Just changed it");
    // only_png("PNG/speed_of_ball_over_time.png");
    // plot(time, speed_of_ball, total_data_points);

    arrfree(height_of_ball);
    arrfree(speed_of_ball);
    arrfree(time);

    return 0;
}
