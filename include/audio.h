#ifndef AUDIO_H
#define AUDIO_H

float audio_calculate_dbfs(float rms);

float audio_normalize_rms(float rms);

int audio_calculate_bar_length(float normalized);

void audio_print_bar(float rms, float dbfs);

#endif // AUDIO_H