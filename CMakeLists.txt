cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(BetterPlotting VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add compile options
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Add linker options
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")

set(CMAKE_UNITY_BUILD TRUE)

# Add definitions for sanitizers
add_compile_options(-fsanitize=address,undefined,leak)
add_link_options(-fsanitize=address,undefined,leak)


# Include directories
include_directories(src/better_version/)

# Add the executable
add_executable(better_plotting src/better_version/better_plotting.cpp)
set_target_properties(better_plotting PROPERTIES UNITY_BUILD_BATCH_SIZE 8)

# Link with SDL2
find_package(SDL2 REQUIRED)
target_link_libraries(better_plotting SDL2::SDL2)

# Debug info
# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-ggdb -Og")
