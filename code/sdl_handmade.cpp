
#include <SDL2/SDL.h>
#include <sys/mman.h> // mmap/munmap

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct offscreen_SDL_buffer
{
    // NOTE(casey): Pixels are always 32-bits wide, Memory Order BB GG RR XX
    SDL_Texture *Texture;
    int Width;
    int Height;
    void *Memory;
    int Pitch;
};

struct sdl_window_dimension
{
    int Width;
    int Height;
};


global_variable bool Running;
global_variable offscreen_SDL_buffer GlobalBackbuffer;

internal sdl_window_dimension
SDLGetWindowSize(SDL_Window *Window) {
    sdl_window_dimension Result = { };
    SDL_GetWindowSize(Window, &Result.Width, &Result.Height);
    return Result;
}

internal void
RenderWeirdGradient(offscreen_SDL_buffer Buffer, int XOffset, int YOffset)
{
    uint8 *Row = (uint8 *)Buffer.Memory;
    for (int Y = 0;
         Y < Buffer.Height;
         Y++)
    {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0;
             X < Buffer.Width;
             X++)
        {
            uint8 Red = 0;
            uint8 Green = Y + YOffset;
            uint8 Blue = X + XOffset;
            *Pixel = (Red << 16) | (Green << 8) | Blue;
            Pixel++;
        }

        Row += Buffer.Pitch;
    }
}

internal void
SDLCopyBufferToRenderer(offscreen_SDL_buffer *Buffer, SDL_Renderer* Renderer)
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
    }
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        SDL_Window *Window = SDL_CreateWindow(
                "Handmade Hero",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                640,
                480,
                SDL_WINDOW_RESIZABLE);

        if (Window)
        {
            SDL_Renderer *Renderer = SDL_CreateRenderer(
                    Window,
                    -1,
                    0);

            if (Renderer)
            {
                sdl_window_dimension WindowSize = SDLGetWindowSize(Window);
                SDLResizeBuffer(&GlobalBackbuffer, SDL_GetWindowID(Window), WindowSize.Width, WindowSize.Height);

                int XOffset = 0;
                int YOffset = 0;
                Running = true;
                while (Running)
                {
                    //NOTE(kjaa): Investigate performance implications of passing Event by value instead of pointer.
                    SDL_Event Event;
                    while (SDL_PollEvent(&Event))
                    {
                        SDLHandleEvent(&Event);
                    }
                    RenderWeirdGradient(GlobalBackbuffer, XOffset, YOffset);
                    SDLCopyBufferToRenderer(&GlobalBackbuffer, Renderer);
                    XOffset++;
                }
            } else
            {
                //TODO(kjaa): Handle CreateRenderer failure
            }
        } else
        {
            //TODO(kjaa): Handle CreateWindow failure
        }
        SDL_Quit();
    } else
    {
        //TODO(kjaa): Handle SDL_Init failure
    }
    return 0;
}
