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

    //SetToneHerz(&SoundOutput, 512 - (int)(256.0f*((real32)StickY / 30000.0f)));
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
GameUpdateAndRender(game_memory *Memory,
                    game_input *Input,
                    game_offscreen_buffer *Buffer,
                    game_sound_output_buffer *SoundBuffer)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize)
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if (!Memory->IsInitialized) 
    {
        GameState->ToneHz = 256;
        GameState->XOffset = 0;
        GameState->YOffset = 0;

        // TODO(Casey): May have to go into the platform layer
        Memory->IsInitialized = true;
    }

    game_controller_input Input0 = Input->Controllers[0];
    if (Input0.IsAnalog)
    {
        GameState->ToneHz = 256 + (int32)(120.0f * (Input0.EndX));
        GameState->YOffset += (int32)(4.0f * Input0.EndY);  
    }
    else
    {
      
    }

    if (Input0.A.EndedDown)
    {
        GameState->XOffset += 1;
    }
  
    GameOutputSound(SoundBuffer, GameState->ToneHz);
    RenderWeirdGradient(Buffer, GameState->XOffset, GameState->YOffset);
}
