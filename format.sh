#!/usr/bin/env bash

set -ex

# Format the code
clang-format -i -style=file $(find . -name '*.cpp' -or -name '*.h')

