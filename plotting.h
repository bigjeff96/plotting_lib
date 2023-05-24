#pragma once
#include <stdlib.h>
enum LEGEND_POSITION { TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, TOP_LEFT, COUNT_LEGEND_POSITION };
enum LINE_TYPE { HLINE, VLINE, LINE_TYPE_COUNT };

void set_y_label(const char *name);
void set_x_label(const char *name);
void set_title(const char *name);
void set_aspect(int width, int height);
void set_legend_position(LEGEND_POSITION legend_position);
void make_submarkings();
void show_legend();
void save_to_png(const char *file_path);
void only_png(const char *file_path);
void draw_perp_line(double value, LINE_TYPE type);
void plot(double x_data[], double y_data[], int total_points, double x_range[2] = NULL, double y_range[2] = NULL);
void scatter_plot(double x_data[], double y_data[], int total_points, double x_range[2] = NULL, double y_range[2] = NULL);
void add_plot(double x_data[], double y_data[], int total_points, const char *legend = NULL);
void plot_all(double x_range[2] = NULL, double y_range[2] = NULL);
void scatter_plot_all(double x_range[2] = NULL, double y_range[2] = NULL);
