
#include <SDL2/SDL.h>
#include <sys/mman.h> // mmap/munmap

#define internal static
#define local_persist static
#define global_variable static

global_variable bool Running;

global_variable SDL_Texture *Texture;

global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable void *BitmapMemory;
global_variable int BytesPerPixel = 4;

internal void
ResizeTexture(uint WindowID, int NewWidth, int NewHeight) {
    //TODO(kjaa): Maybe don't free first, free after realloc succeeds.
    if (Texture) {
        SDL_DestroyTexture(Texture);
    }
    if (BitmapMemory) {
        munmap(BitmapMemory, BitmapWidth * BitmapHeight * BytesPerPixel);
    }

    SDL_Window *Window = SDL_GetWindowFromID(WindowID);
    SDL_Renderer *Renderer = SDL_GetRenderer(Window);
    Texture = SDL_CreateTexture(
            Renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            NewWidth,
            NewHeight
    );
    BitmapMemory = mmap(nullptr,
                  NewWidth * NewHeight * BytesPerPixel,
                  PROT_READ | PROT_WRITE,
                  MAP_ANONYMOUS | MAP_PRIVATE,
                        -1,
                        0);
    if (BitmapMemory == MAP_FAILED) {
        printf("PixelBuffer Alloc failed!");
    }
    BitmapWidth = NewWidth;
    BitmapHeight = NewHeight;
    int Pitch = BitmapWidth;
    int *Row = (int *)BitmapMemory;
    for (int Y = 0;
         Y < BitmapHeight;
         Y++) {
        int *Pixel = Row;
        for (int X = 0;
            X < BitmapWidth;
            X++) {
            *Pixel++ = 0xFFFF0000;
        }

        Row += Pitch;
    }
}

internal void
UpdateWindow(uint WindowID) {
    SDL_Window *Window = SDL_GetWindowFromID(WindowID);
    SDL_Renderer *Renderer = SDL_GetRenderer(Window);

    SDL_UpdateTexture(Texture, nullptr, BitmapMemory, BitmapWidth * 4);
    SDL_RenderCopy(Renderer, Texture, nullptr, nullptr);
    SDL_RenderPresent(Renderer);
}

internal void
HandleEvent(SDL_Event *Event) {
    switch (Event->type) {
        case SDL_QUIT: {
            Running = false;
            break;
        }
        case SDL_WINDOWEVENT: {
            switch (Event->window.event) {
                case SDL_WINDOWEVENT_RESIZED: {
                    printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", Event->window.data1, Event->window.data2);
                    ResizeTexture(Event->window.windowID, Event->window.data1, Event->window.data2);

                    break;
                }
                case SDL_WINDOWEVENT_CLOSE: {
                    Running = false;
                    break;
                }
                case SDL_WINDOWEVENT_EXPOSED: {
                    printf("SDL_WINDOWEVENT_EXPOSED\n");
                    UpdateWindow(Event->window.windowID);
                    break;
                }
            }
            break;
        }
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        SDL_Window *Window = SDL_CreateWindow(
                "Handmade Hero",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                640,
                480,
                SDL_WINDOW_RESIZABLE);

        if (Window) {
            SDL_Renderer *Renderer = SDL_CreateRenderer(
                    Window,
                    -1,
                    0);

            if (Renderer) {
                Running = true;
                while (Running) {
                    SDL_Event Event;
                    if (SDL_WaitEvent(&Event) == 0) {
                        // TODO(kjaa): handle failure
                        break;
                    }
                    HandleEvent(&Event);
                }
            } else {
                //TODO(kjaa): Handle CreateRenderer failure
            }
        } else {
            //TODO(kjaa): Handle CreateWindow failure
        }
        SDL_Quit();
    } else {
        //TODO(kjaa): Handle SDL_Init failure
    }
    return 0;
}
