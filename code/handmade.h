//
// Created by Kristoffer Just Arndal Andersen on 09/06/2022.
//

#if !defined(HANDMADE_H)
#define HANDMADE_H

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

internal void
GameUpdateAndRender(game_offscreen_buffer *Buffer,
                    game_sound_output_buffer *SoundBuffer);

#endif //HANDMADE_H
