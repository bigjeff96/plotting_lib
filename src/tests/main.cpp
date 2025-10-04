#include <cmath>
#include <libplotting/plotting.hpp>
#include <vector>

int main() {
    using namespace std;

    vector<double> x;
    vector<double> y;

    for (int i = 0; i < 200; i++) {
        double x_val = ((double)i / 200.) * 2 * M_PI;
        x.push_back(x_val);
        y.push_back(sin(x_val));
    }

    Plotter plotter;
    LOG("init, but might need to rethink the init function");
    int err = plotter.init(x, y, {}, {}, "test title", "x", "y");
    plotter.x_data = x;
    plotter.y_data = y;
    if (err) exit(1);
    plotter.plot();

    return 0;
}
