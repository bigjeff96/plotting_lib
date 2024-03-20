cmake_minimum_required(VERSION 3.25)

# Set the project name and version
project(BetterPlotting VERSION 1.0)

find_package(SDL2 REQUIRED)

# Add the executable
add_executable(better_plotting
    src/better_version/better_plotting.cpp
    src/better_version/test.cpp
)

# Link with SDL2
target_link_libraries(better_plotting SDL2::SDL2)

target_compile_features(better_plotting PRIVATE cxx_std_20)

target_compile_options(better_plotting
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        $<$<CONFIG:Debug>:-Og -ggdb -fsanitize=address,undefined,leak>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:MinSizeRel>:-Os>
        $<$<CONFIG:RelWithDebInfo>:-O2 -ggdb>
)

target_compile_definitions(better_plotting
    PRIVATE
        "$<$<CONFIG:DEBUG>:DEBUG>"
        "$<$<CONFIG:RELEASE>:RELEASE>"
        "$<$<CONFIG:MINSIZEREL>:MINSIZEREL>"
        "$<$<CONFIG:RELWITHDEBINFO>:RELWITHDEBINFO>"
  )

target_link_options(better_plotting
    PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>
)

target_include_directories(better_plotting
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/better_version
)
