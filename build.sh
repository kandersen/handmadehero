#!/bin/sh
mkdir -p build
pushd build
clang++ `sdl2-config --cflags --libs` -g ../code/sdl_handmade.cpp -o handmadehero
popd
