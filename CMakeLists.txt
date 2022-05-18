cmake_minimum_required(VERSION 3.21)
project(handmadehero)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
add_executable(handmadehero code/sdl_handmade.cpp)
target_link_libraries(handmadehero ${SDL2_LIBRARIES})

