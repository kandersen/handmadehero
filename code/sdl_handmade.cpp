#include <iostream>

#include <SDL2/SDL.h>

int main(int argc, char **argv) {
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Handmade Hero", "This is Handmade Hero", 0);
  std::cout << "Hello" << std::endl;
  return 0;
}
