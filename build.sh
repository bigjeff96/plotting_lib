#!/bin/bash

set -ex
#clang++ -fuse-ld=mold -ffast-math
time {
    clang++ -c -fPIC -I src/ ./src/plotting.cpp  -ggdb -Wall -Wshadow -Wextra -fsanitize=address,undefined,leak 

    clang++ -shared -o libplotting.so plotting.o -ggdb -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_gfx -ggdb -Wall -Wshadow -Wextra -fsanitize=address,undefined,leak -fuse-ld=mold
}
