#ifndef AUDIO_SERVICE_H
#define AUDIO_SERVICE_H

#include <stdbool.h>

typedef struct
{
  double start_wall_time;   // GetTime() when playback is started/resumed
  double accumulated_ms;    // Total elapsed ms
  bool is_playing;
  int total_audio_length;
} AudioClock;

void audio_clock_start(AudioClock *clock);
void audio_clock_pause(AudioClock *clock);
void audio_clock_resume(AudioClock *clock);
float audio_clock_get_time_ms(AudioClock *clock);

#endif // AUDIO_SERVICE_H
