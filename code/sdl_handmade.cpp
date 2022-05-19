
#include <SDL2/SDL.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool Running;
global_variable SDL_Texture *Texture;
global_variable int TextureWidth;
global_variable void *Pixels;

internal void
ResizeTexture(uint WindowID, int Width, int Height) {
    //TODO(kjaa): Maybe don't free first, free after realloc succeeds.
    if (Texture) {
        SDL_DestroyTexture(Texture);
    }
    if (Pixels) {
        free(Pixels);
    }
    SDL_Window *Window = SDL_GetWindowFromID(WindowID);
    SDL_Renderer *Renderer = SDL_GetRenderer(Window);
    Texture = SDL_CreateTexture(
            Renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            Width,
            Height
    );
    TextureWidth = Width;
    Pixels = malloc(Width * Height * 4);
}

internal void
UpdateWindow(uint WindowID) {
    SDL_Window *Window = SDL_GetWindowFromID(WindowID);
    SDL_Renderer *Renderer = SDL_GetRenderer(Window);

    SDL_UpdateTexture(Texture, nullptr, Pixels, TextureWidth * 4);
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
                    ResizeTexture(Event->window.windowID, Event->window.data1, Event->window.data2);
                    printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", Event->window.data1, Event->window.data2);
                    break;
                }
                case SDL_WINDOWEVENT_CLOSE: {
                    Running = false;
                    break;
                }
                case SDL_WINDOWEVENT_EXPOSED: {
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
