
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

global_variable bool Running;

global_variable SDL_Texture *Texture;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable void *BitmapMemory;
global_variable int BytesPerPixel = 4;

internal void
RenderWeirdGradient(int XOffset, int YOffset)
{
    int Pitch = BitmapWidth;
    uint32 *Row = (uint32 *) BitmapMemory;
    for (int Y = 0;
         Y < BitmapHeight;
         Y++)
    {
        uint32 *Pixel = Row;
        for (int X = 0;
             X < BitmapWidth;
             X++)
        {
            uint8 Red = 0;
            uint8 Green = Y + YOffset;
            uint8 Blue = X + XOffset;
            *Pixel = (Red << 16) | (Green << 8) | (Blue);
            Pixel++;
        }

        Row += Pitch;
    }
}

internal void
SDLUpdateRenderer(SDL_Renderer* Renderer)
{
    SDL_UpdateTexture(Texture, nullptr, BitmapMemory, BitmapWidth * 4);
    SDL_RenderCopy(Renderer, Texture, nullptr, nullptr);
    SDL_RenderPresent(Renderer);
}

internal void
SDLResizeTexture(uint WindowID, int NewWidth, int NewHeight)
{
    //TODO(kjaa): Maybe don't free first, free after realloc succeeds.
    if (Texture)
    {
        SDL_DestroyTexture(Texture);
    }
    if (BitmapMemory)
    {
        munmap(BitmapMemory, BitmapWidth * BitmapHeight * BytesPerPixel);
    }

    BitmapWidth = NewWidth;
    BitmapHeight = NewHeight;

    SDL_Window *Window = SDL_GetWindowFromID(WindowID);
    SDL_Renderer *Renderer = SDL_GetRenderer(Window);
    Texture = SDL_CreateTexture(
            Renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            BitmapWidth,
            BitmapHeight
    );
    BitmapMemory = mmap(nullptr,
                        BitmapWidth * BitmapHeight * BytesPerPixel,
                        PROT_READ | PROT_WRITE,
                        MAP_ANONYMOUS | MAP_PRIVATE,
                        -1,
                        0);
    if (BitmapMemory == MAP_FAILED)
    {
        printf("PixelBuffer Alloc failed!");
    }
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
                    SDLResizeTexture(Event->window.windowID, Event->window.data1, Event->window.data2);
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
                int Width, Height;
                SDL_GetWindowSize(Window, &Width, &Height);
                SDLResizeTexture(SDL_GetWindowID(Window), Width, Height);

                int XOffset = 0;
                int YOffset = 0;
                Running = true;
                while (Running)
                {
                    SDL_Event Event;
                    while (SDL_PollEvent(&Event))
                    {
                        SDLHandleEvent(&Event);
                    }
                    RenderWeirdGradient(XOffset, YOffset);
                    SDLUpdateRenderer(Renderer);
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
