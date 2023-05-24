#!/bin/bash
#clang++ -fuse-ld=mold -ffast-math
clang++ `sdl2-config --cflags --libs` -I . render_shapes.cpp plotting.cpp sim_code.cpp rounding_nums.cpp -lSDL2_ttf -lSDL2_image  -lSDL2_gfx -ggdb -Wall -Wshadow 
# echo "Compilation done, now running the program"
./a.out 

