#!/usr/bin/bash

set -ex

time clang++ test/sim_code.cpp -lplotting -L. -Wl,-rpath,'$ORIGIN' -fsanitize=address,undefined,leak -fuse-ld=mold  -ggdb -Wall -Wshadow -Wextra 
