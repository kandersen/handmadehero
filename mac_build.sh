#!/bin/sh

FLAGS=""
FLAGS+=" -fno-cxx-exceptions"
FLAGS+=" -fno-rtti"
FLAGS+=" -fdiagnostics-absolute-paths"
FLAGS+=" -g"
FLAGS+=" -std=c++17"
# FLAGS+=" -O3"


WARNINGS=""
WARNINGS+=" -Wall"
WARNINGS+=" -Wextra"
# WARNINGS+=" -Wpadded"
WARNINGS+=" -Wsign-conversion"
WARNINGS+=" -Wimplicit-int-conversion"
WARNINGS+=" -Wshorten-64-to-32"
# WARNINGS+=" -Weverything"

MUTES=""
MUTES+=" -Wno-unused-function"

DEFINES=""
DEFINES+=" -DHANDMADE_INTERNAL=1"
DEFINES+=" -DHANDMADE_SLOW=1"

INCLUDES=""
INCLUDES+=" `sdl2-config --cflags --libs`"

mkdir -p build
pushd build
clang++ $FLAGS $WARNINGS $MUTES $DEFINES $INCLUDES ../code/sdl_handmade.cpp -o handmadehero
popd
