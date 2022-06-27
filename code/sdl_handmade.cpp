#include <stdint.h>

#define Pi32 3.14159265358979f

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

typedef float real32;
typedef double real64;

// TODO(kjaa): Implement sine ourselves
#include <cmath> // sine

#include "handmade.cpp"

#include <SDL.h>
#include <sys/mman.h> // mmap/munmap
#include <dlfcn.h> // dynamic load of libs: dlopen
#include <x86intrin.h> //rdtsc
#include <sys/types.h> // file IO
#include <sys/stat.h> // file IO
#include <fcntl.h> // file open
#include <unistd.h> // file stat

#include "sdl_handmade.h"

#if HANDMADE_INTERNAL
inline uint32
SafeTruncateUInt64(int64 Value)
{
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32) Value;
    return Result;
}

internal debug_read_file_result
DEBUGPlatformReadEntireFile(char *FileName)
{
    debug_read_file_result Result = {};
    int FileHandle = open(FileName, O_RDONLY);
    if (FileHandle == -1) 
    {
        return Result;
    }

    struct stat FileStatus;
    if (fstat(FileHandle, &FileStatus) == -1)
    {
        close(FileHandle);
        return Result;
    }

    Result.ContentSize = SafeTruncateUInt64(FileStatus.st_size);
    Result.Content = malloc(Result.ContentSize);
    if (!Result.Content) {
        Result.ContentSize = 0;
        close(FileHandle);
        return Result;
    }

    uint32 BytesToRead = Result.ContentSize;
    uint8 *NextByteLocation = (uint8*)Result.Content;
    while (BytesToRead)
    {
        ssize_t BytesRead = read(FileHandle, NextByteLocation, BytesToRead);
        if (BytesRead == -1)
        {
            free(Result.Content);
            Result.Content = 0;
            Result.ContentSize = 0;
            close(FileHandle);
            return Result;            
        }
        BytesToRead -= BytesRead;
        NextByteLocation += BytesRead;
    }

    close(FileHandle);
    return Result;
}

internal void
DEBUGPlatformFreeFileMemory(void *Memory)
{
    free(Memory);
}

internal bool32
DEBUGPlatformWriteEntireFile(char *FileName, uint32 MemorySize, void *Memory)
{
    int FileHandle = open(FileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (FileHandle == -1)
    {
        return false;
    }

    uint32 BytesToWrite = MemorySize;
    uint8 *NextByteLocation = (uint8*)Memory;
    while (BytesToWrite)
    {
        uint32 BytesWritten = write(FileHandle, NextByteLocation, BytesToWrite);
        if (BytesWritten)
        {
            close(FileHandle);
            return false;
        }
        BytesToWrite -= BytesWritten;
        NextByteLocation += BytesWritten;
    }

    close(FileHandle);
    return true;
}
#else
#endif

global_variable bool32 Running;
global_variable bool32 SoundIsPlaying;
global_variable offscreen_SDL_buffer GlobalBackbuffer;

internal sdl_window_dimension
SDLGetWindowSize(SDL_Window *Window)
{
    sdl_window_dimension Result = {};
    SDL_GetWindowSize(Window, &Result.Width, &Result.Height);
    return Result;
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
SDLProcessKeyPress(game_button_state *NewState,
                   bool32 IsDown)
{
    Assert(NewState->EndedDown != IsDown);
    NewState->EndedDown = IsDown;
    ++NewState->HalfTransitionCount;
} 

internal void
SDLHandleEvents(game_controller_input *KeyboardController)
{
                            // NOTE(casey): Investigate performance implications of passing Event by value instead of pointer.
    SDL_Event Event;
    while (SDL_PollEvent(&Event))
    {
        switch (Event.type)
        {
            case SDL_QUIT:
            {
                Running = false;
                break;
            }

            case SDL_WINDOWEVENT:
            {
                switch (Event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        break;
                    }

                    case SDL_WINDOWEVENT_CLOSE:
                    {
                        Running = false;
                        break;
                    }

                    case SDL_WINDOWEVENT_EXPOSED:
                    {
                        break;
                    }
                }
                break;
            }

            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                SDL_Keycode KeyCode = Event.key.keysym.sym;
                bool32 WasDown = Event.key.state == SDL_RELEASED || Event.key.repeat != 0;
                bool32 IsDown = Event.key.state == SDL_PRESSED;

                if (WasDown != IsDown)
                {
                    if (KeyCode == SDLK_w)
                    {
                       SDLProcessKeyPress(&KeyboardController->MoveUp, IsDown);
                    }
                    else if (KeyCode == SDLK_a)
                    {
                       SDLProcessKeyPress(&KeyboardController->MoveLeft, IsDown);
                    }
                    else if (KeyCode == SDLK_s)
                    {
                       SDLProcessKeyPress(&KeyboardController->MoveDown, IsDown);
                    }
                    else if (KeyCode == SDLK_d)
                    {
                       SDLProcessKeyPress(&KeyboardController->MoveRight, IsDown);
                    }
                    else if (KeyCode == SDLK_q)
                    {
                       SDLProcessKeyPress(&KeyboardController->LeftShoulder, IsDown);
                    }
                    else if (KeyCode == SDLK_e)
                    {
                       SDLProcessKeyPress(&KeyboardController->RightShoulder, IsDown);
                    }
                    else if (KeyCode == SDLK_UP)
                    {
                       SDLProcessKeyPress(&KeyboardController->ActionUp, IsDown);
                    }
                    else if (KeyCode == SDLK_LEFT)
                    {
                       SDLProcessKeyPress(&KeyboardController->ActionLeft, IsDown);
                    }
                    else if (KeyCode == SDLK_DOWN)
                    {
                       SDLProcessKeyPress(&KeyboardController->ActionDown, IsDown);
                    }
                    else if (KeyCode == SDLK_RIGHT)
                    {
                       SDLProcessKeyPress(&KeyboardController->ActionRight, IsDown);
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
            break;
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
SDLInitSDLAudio(sdl_sound_output *SoundOutput)
{
    // TODO(kjaa): Error handling in... any of this.
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0)
    {
        SDL_AudioSpec AudioSpecRequest = {}, AudioSpecReceive;
        AudioSpecRequest.freq = SoundOutput->SamplesPerSecond;
        AudioSpecRequest.format = AUDIO_S16LSB;
        AudioSpecRequest.channels = 2;

        SDL_AudioDeviceID AudioDeviceID = SDL_OpenAudioDevice(
                nullptr,
                SDL_FALSE,
                &AudioSpecRequest,
                &AudioSpecReceive,
                SDL_AUDIO_ALLOW_ANY_CHANGE);

        if (AudioDeviceID != 0 && AudioSpecReceive.format == AudioSpecRequest.format) {
            SoundOutput->DeviceID = AudioDeviceID;
        }
        else
        {
            // TODO(kjaa): Logging
            printf("Audio Init failed");
        }
    }
    else
    {
        // TODO(kjaa): Logging
        printf("Audio Init failed");
    }
}

internal void
SDLFillSoundBuffer(sdl_sound_output *SoundOutput, uint32 BytesToWrite, game_sound_output_buffer *SoundBuffer) {
    SDL_QueueAudio(SoundOutput->DeviceID, SoundBuffer->Samples, BytesToWrite);

    if (!SoundIsPlaying) {
        SDL_PauseAudioDevice(SoundOutput->DeviceID, 0);
        SoundIsPlaying = true;
    }
}

internal void
SDLProcessDigitalButton(bool32 Value,
                        game_button_state *OldState,
                        game_button_state *NewState)
{
    NewState->EndedDown = Value;
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}   

internal real32
SDLProcessControllerStick(SDL_GameController *Controller, SDL_GameControllerAxis Axis, int16 Deadzone)
{
    real32 Result = 0.0f;

    int16 Value = SDL_GameControllerGetAxis(Controller, Axis);
    if (Value < -Deadzone) {
        Result = (real32)Value / 32768.0f;
    } else if (Value > Deadzone) {
        Result = (real32)Value / 32767.0f;
    }

    return Result;
}

int main()
{
#if 0
    uint64 PerfCounterFrequency = SDL_GetPerformanceFrequency();
#endif 
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) == 0)
    {

//        SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
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
            #if HANDMADE_INTERNAL
            void *BaseAddress = (void*)Terabytes(2);
            #else
            void *BaseAddress = 0;
            #endif

            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabytes(64);
            GameMemory.TransientStorageSize = Gigabytes(4);
            GameMemory.PermanentStorage =  mmap(BaseAddress,
                                                GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize,
                                                PROT_READ | PROT_WRITE,
                                                MAP_ANONYMOUS | MAP_PRIVATE,
                                                -1,
                                                0);
            GameMemory.TransientStorage = (uint8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize;

            sdl_sound_output SoundOutput = {};
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.Samples = (int16 *)mmap(nullptr,
                                                SoundOutput.SamplesPerSecond,
                                                PROT_READ | PROT_WRITE,
                                                MAP_ANONYMOUS | MAP_PRIVATE,
                                                -1,
                                                0);
            SoundOutput.BytesPerSample = sizeof(int16) * 2;
            // TODO(kjaa): Can this be dynamic instead of hardwired?
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 11;

            if (GameMemory.PermanentStorage && SoundOutput.Samples)
            {

                SDLInitSDLAudio(&SoundOutput);

                if (SoundOutput.DeviceID == 0)
                {
                    // TODO(kjaa): Sound Initialization error handling
                }

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
                    SDL_GameController *ControllerHandles[MAX_CONTROLLERS];

                    int MaxJoysticks = SDL_NumJoysticks();
                    int OpenControllers = 1;
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
                        OpenControllers++;
                    }


                    SoundIsPlaying = false;

                    game_input Input[2] = {};
                    game_input *NewInput = &Input[0];
                    game_input *OldInput = &Input[1];

                    Running = true;
                    uint64 LastCounter = SDL_GetPerformanceCounter();
                    uint64 LastCycleCount = _rdtsc();
                    while (Running)
                    {
                        // NOTE(kjaa): Zero the new keyboard controller transition count, and preserve EndedDown
                        game_controller_input *OldKeyboardController = &OldInput->Controllers[0];                        
                        game_controller_input *NewKeyboardController = &NewInput->Controllers[0];
                        *NewKeyboardController = {};
                        NewKeyboardController->IsConnected = true;
                        for (int ButtonIndex = 0;
                             ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
                             ++ButtonIndex)
                        {
                            NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                                OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                        }

                        SDLHandleEvents(NewKeyboardController);

                        // TODO(casey): Should we poll this more frequently (kjaa: than the render loop...?)
                        // TODO(kjaa): handle OpenControllers higher than the hardcoded count in game_input.Controllers
                        for (int ControllerIndex = 1;
                            ControllerIndex < OpenControllers;
                            ControllerIndex++)
                        {                     
                            SDL_GameController *Controller = ControllerHandles[ControllerIndex];                        
                            game_controller_input *OldController = &OldInput->Controllers[ControllerIndex];
                            game_controller_input *NewController = &NewInput->Controllers[ControllerIndex];
                            
                            if (SDL_GameControllerGetAttached(Controller))
                            {
                                NewController->IsConnected = true;

                                NewController->IsAnalog = true;
                                #define CONTROLLER_DEADZONE 7849
                                NewController->StickAverageX = SDLProcessControllerStick(Controller, 
                                                                     SDL_CONTROLLER_AXIS_LEFTX,
                                                                     CONTROLLER_DEADZONE);
                                NewController->StickAverageY = SDLProcessControllerStick(Controller, 
                                                                     SDL_CONTROLLER_AXIS_LEFTY,
                                                                     CONTROLLER_DEADZONE);

                                if (SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_UP))
                                {
                                    NewController->StickAverageY = 1.0f;
                                }
                                if (SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
                                {
                                    NewController->StickAverageY = -1.0f;
                                }
                                if (SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
                                {
                                    NewController->StickAverageX = -1.0f;
                                }
                                if (SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
                                {
                                    NewController->StickAverageX = 1.0f;
                                }
                                


                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_UP),
                                                        &OldController->MoveUp,
                                                        &NewController->MoveUp);
                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN),
                                                        &OldController->MoveDown,
                                                        &NewController->MoveDown);
                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT),
                                                        &OldController->MoveLeft,
                                                        &NewController->MoveLeft);
                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT),
                                                        &OldController->MoveRight,
                                                        &NewController->MoveRight);                                                                    
                             
                                SDLProcessDigitalButton(NewController->StickAverageX < 0.5f,
                                                        &OldController->MoveLeft,
                                                        &NewController->MoveLeft);
                                SDLProcessDigitalButton(NewController->StickAverageX > 0.5f,
                                                        &OldController->MoveRight,
                                                        &NewController->MoveRight);
                                SDLProcessDigitalButton(NewController->StickAverageY < 0.5f,
                                                        &OldController->MoveDown,
                                                        &NewController->MoveDown);
                                SDLProcessDigitalButton(NewController->StickAverageX > 0.5f,
                                                        &OldController->MoveUp,
                                                        &NewController->MoveUp);
                                

                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_A),
                                                        &OldController->ActionDown,
                                                        &NewController->ActionDown);
                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_B),
                                                        &OldController->ActionRight,
                                                        &NewController->ActionRight);
                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_X),
                                                        &OldController->ActionLeft,
                                                        &NewController->ActionLeft);
                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_Y),
                                                        &OldController->ActionUp,
                                                        &NewController->ActionUp);
                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER),
                                                        &OldController->LeftShoulder,
                                                        &NewController->LeftShoulder);
                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER),
                                                        &OldController->RightShoulder,
                                                        &NewController->RightShoulder);
                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_START),
                                                        &OldController->Start,
                                                        &NewController->Start);
                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_BACK),
                                                        &OldController->Back,
                                                        &NewController->Back);
                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_GUIDE),
                                                        &OldController->Guide,
                                                        &NewController->Guide);
                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_LEFTSTICK),
                                                        &OldController->LeftStick,
                                                        &NewController->LeftStick);
                                SDLProcessDigitalButton(SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK),
                                                        &OldController->RightStick,
                                                        &NewController->RightStick);
                            }
                            else
                            {
                                // NOTE(casey): The controller is not available
                            };
                        }

                        int32 TargetQueueBytes = SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample;
                        uint32 BytesToWrite = TargetQueueBytes - SDL_GetQueuedAudioSize(SoundOutput.DeviceID);
                        game_sound_output_buffer SoundBuffer = {};
                        SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                        SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                        SoundBuffer.Samples = SoundOutput.Samples;

                        game_offscreen_buffer Buffer = {};
                        Buffer.Memory = GlobalBackbuffer.Memory;
                        Buffer.Width = GlobalBackbuffer.Width;
                        Buffer.Height = GlobalBackbuffer.Height;
                        Buffer.Pitch = GlobalBackbuffer.Pitch;

                        GameUpdateAndRender(&GameMemory, NewInput, &Buffer, &SoundBuffer);

                        SDLCopyBufferToRenderer(&GlobalBackbuffer, Renderer);
                        SDLFillSoundBuffer(&SoundOutput, BytesToWrite, &SoundBuffer);


                        uint64 EndCycleCount = _rdtsc();
                        uint64 EndCounter = SDL_GetPerformanceCounter();
#if 0
                        uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
                        uint64 CounterElapsed = EndCounter - LastCounter;
                        real64 MSPerFrame = (1000.0f * (real64)CounterElapsed) / (real64)PerfCounterFrequency;
                        real64 FPS = (real64)PerfCounterFrequency / (real64)CounterElapsed;
                        uint64 MCPF = CyclesElapsed / (1000 * 1000);
                        char PBuffer[256];
                        snprintf(PBuffer, sizeof(PBuffer), "%.02fms/f, %.02fFPS, %lldmc/f\n", MSPerFrame, FPS, MCPF);
                        printf("%s", PBuffer);
#endif
                        LastCycleCount = EndCycleCount;
                        LastCounter = EndCounter;

                        game_input *Temp = NewInput;
                        NewInput = OldInput;
                        OldInput = Temp;
                    }
                }
                else
                {
                    //TODO(kjaa): Handle CreateRenderer failure
                }
            }
            
            else
            {
               // TODO(kjaa): Handle memory init failure
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
