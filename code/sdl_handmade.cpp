
#include <SDL2/SDL.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool Running;

void HandleEvent(SDL_Event *Event) {
    switch (Event->type) {
        case SDL_QUIT: {
            Running = false;
        }
            break;
        case SDL_WINDOWEVENT: {
            switch (Event->window.event) {
                case SDL_WINDOWEVENT_RESIZED: {
                    printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", Event->window.data1, Event->window.data2);
                }
                    break;
                case SDL_WINDOWEVENT_CLOSE: {
                    Running = false;
                }
                    break;
                case SDL_WINDOWEVENT_EXPOSED: {
                    SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
                    SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                    local_persist bool IsWhite = true;
                    if (IsWhite) {
                        SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
                        IsWhite = false;
                    } else {
                        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
                        IsWhite = true;
                    }
                    SDL_RenderClear(Renderer);
                    SDL_RenderPresent(Renderer);
                }
                    break;
            }
        }
            break;

    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        SDL_Window *Window;
        Window = SDL_CreateWindow(
                "Handmade Hero",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                640,
                480,
                SDL_WINDOW_RESIZABLE
        );

        if (Window) {
            int autodetect_driver = -1;
            int renderer_flags = 0;
            SDL_Renderer *Renderer = SDL_CreateRenderer(
                    Window,
                    autodetect_driver,
                    renderer_flags
            );

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
