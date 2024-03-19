#include <string>

struct Plotter {
    std::string title;
    std::string x_label;
    std::string y_label;
    int width;
    int height;
};

int plot_func();
