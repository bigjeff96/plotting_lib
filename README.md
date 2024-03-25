# plotting_lib

Matplotlib clone for c++

## Getting Started

Clone the repository:

```sh
git clone 'https://github.com/bigjeff96/plotting_lib'
cd plotting_lib
```

Configure the project:

```sh
cmake -S. -Bbuild
```

This creates a `build` folder from the `.` source directory.
You can specify some options, for exemple the build type:

```sh
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Debug
```

Build the project:

```sh
cmake --build build
```

Run the test binary:

```sh
./build/plotting-tests
```

When building in debug mode, Leak Sanitizer is enabled. Run the binary using the
following to avoid reporting leaks from third-party:

```sh
LSAN_OPTIONS=suppressions=ignore.lsan ./build/plotting-tests
```
