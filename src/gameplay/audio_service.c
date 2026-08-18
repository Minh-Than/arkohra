#include "audio_service.h"
#include "raylib.h"

void audio_clock_start(AudioClock *clock)
{
  clock->accumulated_ms = 0.0;
  clock->start_wall_time = GetTime();
  clock->is_playing = true;
}

void audio_clock_pause(AudioClock *clock)
{
  if (!clock->is_playing) return;
  clock->accumulated_ms += (GetTime() - clock->start_wall_time) * 1000.0;
  clock->is_playing = false;
}

void audio_clock_resume(AudioClock *clock)
{
  clock->start_wall_time = GetTime();
  clock->is_playing = true;
}

float audio_clock_get_time_ms(AudioClock *clock)
{
  if (!clock->is_playing) return clock->accumulated_ms;
  return (float)(clock->accumulated_ms + (GetTime() - clock->start_wall_time) * 1000.0);
}
