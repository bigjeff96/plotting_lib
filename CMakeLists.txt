cmake_minimum_required(VERSION 3.25)

# Set the project name and version
project(BetterPlotting VERSION 1.0)

find_package(SDL2 REQUIRED)

# Add a static library
add_library(plotting
    src/libplotting/plotting.cpp
    src/libplotting/render_shapes.cpp
)

target_link_libraries(plotting
    PUBLIC
        SDL2::SDL2
)

target_compile_features(plotting
    PUBLIC
        cxx_std_20
)

target_compile_options(plotting
    PUBLIC
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        "$<$<CONFIG:Debug>:-Og>"
        "$<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>"
        "$<$<CONFIG:MinSizeRel>:-Os>"
        "$<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:-O2>"
        "$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-ggdb>"
)

target_link_options(plotting
    PUBLIC
        "$<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>"
)

target_include_directories(plotting
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src
)

# Add the executable
add_executable(plotting-tests
    src/tests/main.cpp
)

target_link_libraries(plotting-tests
    PRIVATE
        plotting
)
