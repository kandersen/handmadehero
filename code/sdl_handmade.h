
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

struct sdl_sound_output {
  SDL_AudioDeviceID DeviceID;
  int16 *Samples;
  int SamplesPerSecond;
  int BytesPerSample;
  int LatencySampleCount;
};


