
#include <SDL.h>
#include <sys/mman.h> // mmap/munmap
#include <dlfcn.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct offscreen_SDL_buffer
{
    // NOTE(casey): Pixels are always 32-bits wide, Memory Order BB GG RR XX
    SDL_Texture *Texture;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct sdl_window_dimension
{
    int Width;
    int Height;
};


global_variable bool32 Running;
global_variable offscreen_SDL_buffer GlobalBackbuffer;

internal sdl_window_dimension
SDLGetWindowSize(SDL_Window *Window)
{
    sdl_window_dimension Result = {};
    SDL_GetWindowSize(Window, &Result.Width, &Result.Height);
    return Result;
}

internal void
RenderWeirdGradient(offscreen_SDL_buffer *Buffer, int XOffset, int YOffset)
{
    uint8 *Row = (uint8 *) Buffer->Memory;
    for (int Y = 0;
         Y < Buffer->Height;
         Y++)
    {
        uint32 *Pixel = (uint32 *) Row;
        for (int X = 0;
             X < Buffer->Width;
             X++)
        {
            uint8 Red = 0;
            uint8 Green = Y + YOffset;
            uint8 Blue = X + XOffset;
            *Pixel = (Red << 16) | (Green << 8) | Blue;
            Pixel++;
        }

        Row += Buffer->Pitch;
    }
}

internal void
SDLCopyBufferToRenderer(offscreen_SDL_buffer *Buffer, SDL_Renderer *Renderer)
{
    SDL_UpdateTexture(Buffer->Texture, nullptr, Buffer->Memory, Buffer->Width * 4);
    SDL_RenderCopy(Renderer, Buffer->Texture, nullptr, nullptr);
    SDL_RenderPresent(Renderer);
}

internal void
SDLResizeBuffer(offscreen_SDL_buffer *Buffer, uint WindowID, int NewWidth, int NewHeight)
{
    //TODO(kjaa): Maybe don't free first, free after realloc succeeds.
    if (Buffer->Texture)
    {
        SDL_DestroyTexture(Buffer->Texture);
    }
    if (Buffer->Memory)
    {
        munmap(Buffer->Memory, Buffer->Pitch * Buffer->Height);
    }

    const int BytesPerPixel = 4;
    Buffer->Width = NewWidth;
    Buffer->Height = NewHeight;
    Buffer->Pitch = Buffer->Width * BytesPerPixel;

    SDL_Window *Window = SDL_GetWindowFromID(WindowID);
    SDL_Renderer *Renderer = SDL_GetRenderer(Window);
    Buffer->Texture = SDL_CreateTexture(
            Renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            Buffer->Width,
            Buffer->Height
    );

    Buffer->Memory = mmap(nullptr,
                          Buffer->Pitch * Buffer->Height,
                          PROT_READ | PROT_WRITE,
                          MAP_ANONYMOUS | MAP_PRIVATE,
                          -1,
                          0);
}


internal void
SDLHandleEvent(SDL_Event *Event)
{
    switch (Event->type)
    {
        case SDL_QUIT:
        {
            Running = false;
            break;
        }

        case SDL_WINDOWEVENT:
        {
            switch (Event->window.event)
            {
                case SDL_WINDOWEVENT_RESIZED:
                {
                    printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", Event->window.data1, Event->window.data2);
                    break;
                }

                case SDL_WINDOWEVENT_CLOSE:
                {
                    Running = false;
                    break;
                }

                case SDL_WINDOWEVENT_EXPOSED:
                {
                    printf("SDL_WINDOWEVENT_EXPOSED\n");
                    break;
                }
            }
            break;
        }

        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            SDL_Keycode KeyCode = Event->key.keysym.sym;
            bool WasDown = Event->key.state == SDL_RELEASED || Event->key.repeat != 0;
            bool IsDown = Event->key.state == SDL_PRESSED;

            if (WasDown != IsDown)
            {
                if (KeyCode == SDLK_w)
                {
                    printf("W: ");
                    if (IsDown)
                    {
                        printf("IsDown ");
                    }
                    if (WasDown)
                    {
                        printf("WasDown ");
                    }
                    printf("\n");
                }
                else if (KeyCode == SDLK_a)
                {
                }
                else if (KeyCode == SDLK_s)
                {
                }
                else if (KeyCode == SDLK_d)
                {
                }
                else if (KeyCode == SDLK_q)
                {
                }
                else if (KeyCode == SDLK_e)
                {
                }
                else if (KeyCode == SDLK_UP)
                {
                }
                else if (KeyCode == SDLK_LEFT)
                {
                }
                else if (KeyCode == SDLK_DOWN)
                {
                }
                else if (KeyCode == SDLK_RIGHT)
                {
                }
                else if (KeyCode == SDLK_ESCAPE)
                {
                    Running = false;
                }
                else if (KeyCode == SDLK_SPACE)
                {
                }
            }
        }
    }
}

void DynamicLoad()
{
    void *LibHandle = dlopen("libSDL2.dylib", RTLD_NOW);
    void *ProcHandle = dlsym(LibHandle, "SDL_Init");
    dlclose(LibHandle);

    LibHandle = SDL_LoadObject("libSDL2.dylib");
    ProcHandle = SDL_LoadFunction(LibHandle, "SDL_Init");
    SDL_UnloadObject(LibHandle);
}

internal void
SDLAudioCallback(void *UserData, uint8 *AudieData, int Length)
{
    memset(AudieData, 0, Length);
}

internal void
SDLInitSDLAudio(int32 SamplesPerSecond)
{
    // TODO(kjaa): Error handling in... any of this.
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0)
    {
        SDL_AudioSpec AudioSettings = {};
        AudioSettings.freq = SamplesPerSecond;
        AudioSettings.format = AUDIO_S16LSB;
        AudioSettings.channels = 2;
        AudioSettings.samples = SamplesPerSecond * sizeof(int16);
        AudioSettings.callback = &SDLAudioCallback;
        SDL_OpenAudio(&AudioSettings, nullptr);
    }
    else
    {
        // TODO(kjaa): Logging
        printf("Audio Init failed");
    }
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) == 0)
    {
        SDL_Window *Window = SDL_CreateWindow(
                "Handmade Hero",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                640,
                480,
                SDL_WINDOW_RESIZABLE);

        // TODO(kjaa): Move audio init after window init
        if (Window)
        {
            SDLInitSDLAudio(48000);

            SDL_Renderer *Renderer = SDL_CreateRenderer(
                    Window,
                    -1,
                    0);

            if (Renderer)
            {
                sdl_window_dimension WindowSize = SDLGetWindowSize(Window);
                SDLResizeBuffer(&GlobalBackbuffer, SDL_GetWindowID(Window), WindowSize.Width, WindowSize.Height);

                // TODO(kjaa): Move the controller opening logic into a helper.
                // TODO(kjaa): This is not robust to controller changes. Handle
                //   SDL_CONTROLLERDEVICEADDED, SDL_CONTROLLERDEVICEREMOVED and
                //   SDL_CONTROLLERDEVICEREMAPPED events in the event handler.
#define MAX_CONTROLLERS 4
                SDL_GameController *ControllerHandles[MAX_CONTROLLERS];
                int MaxJoysticks = SDL_NumJoysticks();
                int OpenControllers = 0;
                for (int JoystickIndex = 0; JoystickIndex < MaxJoysticks; JoystickIndex++)
                {
                    if (!SDL_IsGameController(JoystickIndex))
                    {
                        continue;
                    }
                    else if (OpenControllers >= MAX_CONTROLLERS)
                    {
                        break;
                    }
                    ControllerHandles[OpenControllers] = SDL_GameControllerOpen(JoystickIndex);
                    SDL_Joystick *Joystick = SDL_GameControllerGetJoystick(ControllerHandles[OpenControllers]);
                    OpenControllers++;
                }

                SDL_PauseAudio(0);

                int XOffset = 0;
                int YOffset = 0;
                Running = true;
                while (Running)
                {
                    // NOTE(casey): Investigate performance implications of passing Event by value instead of pointer.
                    SDL_Event Event;
                    while (SDL_PollEvent(&Event))
                    {
                        SDLHandleEvent(&Event);
                    }

                    // TODO(casey): Should we poll this more frequently (kjaa: than the render loop...?)
                    for (int ControllerIndex = 0;
                         ControllerIndex < OpenControllers;
                         ControllerIndex++)
                    {
                        SDL_GameController *Controller = ControllerHandles[ControllerIndex];
                        if (SDL_GameControllerGetAttached(Controller))
                        {
                            bool32 Up = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
                            bool32 Down = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
                            bool32 Left = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
                            bool32 Right = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
                            bool32 Start = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_START);
                            bool32 Back = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_BACK);
                            bool32 AButton = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_A);
                            bool32 BButton = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_B);
                            bool32 XButton = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_X);
                            bool32 YButton = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_Y);
                            bool32 LeftShoulder = SDL_GameControllerGetButton(Controller,
                                                                              SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
                            bool32 RightShoulder = SDL_GameControllerGetButton(Controller,
                                                                               SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
                            bool32 LeftStick = SDL_GameControllerGetButton(Controller,
                                                                           SDL_CONTROLLER_BUTTON_LEFTSTICK);
                            bool32 RightStick = SDL_GameControllerGetButton(Controller,
                                                                            SDL_CONTROLLER_BUTTON_RIGHTSTICK);
                            bool32 XBoxButton = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_GUIDE);

                            int16 StickX = SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_LEFTX);
                            int16 StickY = SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_LEFTY);

                            if (Up)
                            {
                                YOffset += 2;
                            }
                            else if (Down)
                            {
                                YOffset -= 2;
                            }

                            if (Left)
                            {
                                XOffset += 2;
                            }
                            else if (Right)
                            {
                                XOffset -= 2;
                            }
                        }
                        else
                        {
                            // NOTE(casey): The controller is not available
                        };
                    }

                    RenderWeirdGradient(&GlobalBackbuffer, XOffset, YOffset);
                    SDLCopyBufferToRenderer(&GlobalBackbuffer, Renderer);
                    XOffset++;
                }
            }
            else
            {
                //TODO(kjaa): Handle CreateRenderer failure
            }
        }
        else
        {
            //TODO(kjaa): Handle CreateWindow failure
        }
        SDL_Quit();

    }
    else
    {
        //TODO(kjaa): Handle SDL_Init failure
    }
    return 0;
}
