//
// Created by Kristoffer Just Arndal Andersen on 09/06/2022.
//

#if !defined(HANDMADE_H)
#define HANDMADE_H

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

// NOTE(casey): Services that the platform layer provides to the game


// NOTE(casey): Services that the game provides to the platform layer

struct game_offscreen_buffer
{
    // NOTE(casey): Pixels are always 32-bits wide, Memory Order BB GG RR XX
    void *Memory;
    int32 Width;
    int32 Height;
    int32 Pitch;
};

struct game_sound_output_buffer
{
    uint32 SampleCount;
    int32 SamplesPerSecond;
    int16 *Samples;
};

struct game_button_state
{
    int32 HalfTransitionCount;
    bool32 EndedDown;
};

struct game_controller_input
{
    bool32 IsAnalog;
    
    real32 StartX;
    real32 StartY;

    real32 MinX;
    real32 MinY;

    real32 MaxX;
    real32 MaxY;
    
    real32 EndX;
    real32 EndY;
    union
    {
        game_button_state Buttons[15];
        struct
        {
            game_button_state Up;
            game_button_state Down;
            game_button_state Left;
            game_button_state Right;
            game_button_state LeftShoulder;
            game_button_state RightShoulder;
            game_button_state LeftStick;
            game_button_state RightStick;
            game_button_state A;
            game_button_state B;
            game_button_state X;
            game_button_state Y;
            game_button_state Start;
            game_button_state Back;
            game_button_state Guide;
        };
    };
};

struct game_input
{
    game_controller_input Controllers[4];
};

internal void
GameUpdateAndRender(game_input *Input,
                    game_offscreen_buffer *Buffer,
                    game_sound_output_buffer *SoundBuffer);

#endif //HANDMADE_H
