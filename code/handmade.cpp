#include "handmade.h"

internal void
GameOutputSound(game_sound_output_buffer *SoundBuffer, int32 ToneHerz) {
    local_persist real32 tSine = 0.0f;
    int16 ToneVolume = 3000;
    int32 WavePeriod = SoundBuffer->SamplesPerSecond/ToneHerz;

    int16 *SampleOut = (int16 *) SoundBuffer->Samples;

    for (uint32 SampleIndex = 0;
         SampleIndex < SoundBuffer->SampleCount;
         ++SampleIndex)
    {
        real32 SineValue = sinf(tSine);
        int16 Sample = (int16) (SineValue * (real32) ToneVolume);
        *SampleOut++ = Sample;
        *SampleOut++ = Sample;

        tSine += 2.0f * Pi32 * 1.0f / (real32) WavePeriod;

    }
}

internal void
RenderWeirdGradient(game_offscreen_buffer *Buffer, int32 XOffset, int32 YOffset)
{
    uint8 *Row = (uint8 *) Buffer->Memory;
    for (int32 Y = 0;
         Y < Buffer->Height;
         Y++)
    {
        uint32 *Pixel = (uint32 *) Row;
        for (int32 X = 0;
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
GameUpdateAndRender(game_offscreen_buffer *Buffer, game_sound_output_buffer *SoundBuffer, int32 ToneHerz) {
    GameOutputSound(SoundBuffer, ToneHerz);
    RenderWeirdGradient(Buffer , 0, 0);
}
