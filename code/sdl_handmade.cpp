
#include <SDL2/SDL.h>

/**
 * @return true, if quit was signaled
 */

bool HandleEvent(SDL_Event *Event) {
    bool ShouldQuit = false;
    switch (Event->type) {
        case SDL_QUIT: {
            ShouldQuit = true;
        }
            break;
        case SDL_WINDOWEVENT: {
            switch (Event->window.event) {
                case SDL_WINDOWEVENT_RESIZED: {
                    printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", Event->window.data1, Event->window.data2);
                } break;
                case SDL_WINDOWEVENT_CLOSE: {
                    ShouldQuit = true;
                } break;
                case SDL_WINDOWEVENT_EXPOSED: {
                    SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
                    SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                    static bool IsWhite = true;
                    if (IsWhite) {
                        SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
                        IsWhite = false;
                    } else {
                        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
                        IsWhite = true;
                    }
                    SDL_RenderClear(Renderer);
                    SDL_RenderPresent(Renderer);
                } break;
            }
        }
            break;

    }
    return ShouldQuit;
}

void ShowMessageBox() {
    SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_INFORMATION,
            "Handmade Hero",
            "This is Handmade Hero",
            nullptr);
}

int main() {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        // TODO(kjaa): handle failure
    }


    SDL_Window *Window;
    Window = SDL_CreateWindow(
            "Handmade Hero",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            640,
            480,
            SDL_WINDOW_RESIZABLE
    );

    if (!Window) {
        // TODO(kjaa): handle failure
    }

    int autodetect_driver = -1;
    int renderer_flags = 0;
    SDL_Renderer *Renderer = SDL_CreateRenderer(
            Window,
            autodetect_driver,
            renderer_flags
    );

    if (!Renderer) {
        // TODO(kjaa): handle failure
    }

    for (;;) {
        SDL_Event Event;
        if (SDL_WaitEvent(&Event) == 0) {
            // TODO(kjaa): handle failure
            break;
        }

        if (HandleEvent(&Event)) {
            break;
        }
    }
    
    SDL_Quit();
    return 0;
}
