#!/bin/sh
mkdir -p build
pushd build
clang++ -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 `sdl2-config --cflags --libs` -fdiagnostics-absolute-paths -Wall -Wextra -g ../code/sdl_handmade.cpp -o handmadehero
popd
