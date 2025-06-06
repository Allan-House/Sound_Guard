#include "audio.h"
#include "config.h"

#include <stdio.h>
#include <math.h>

float audio_normalize_rms(float rms) {
    const float rms_scale = 1.0f / MAX_RMS;

    float normalized = rms * rms_scale;
    if (normalized > 1.0f) {
        normalized = 1.0f;
    }

    return normalized;
}

float audio_calculate_dbfs(float rms) {
    const float rms_scale = 1.0f / MAX_RMS;
    const float log_scale = 20.0f / log(10.0f);

    return log_scale * logf((rms > 0.0001f ? rms : 0.0001f) * rms_scale);
}

int audio_calculate_bar_length(float normalized) {
    float norm_clamped = (normalized < MIN_NORMALIZED) ? MIN_NORMALIZED : normalized;
    int bar_length = (int)((logf(norm_clamped) - logf(MIN_NORMALIZED)) / (-logf(MIN_NORMALIZED)) * BAR_WIDTH);

    if (bar_length < 0) {
        bar_length = 0;
    }
    
    if (bar_length > BAR_WIDTH) {
        bar_length = BAR_WIDTH;
    }
    
    return bar_length;
}

void audio_print_bar(float rms, float dbfs) {
    float normalized = audio_normalize_rms(rms);
    int bar_length = audio_calculate_bar_length(normalized);
    
    printf("Volume: ");
    for (int i = 0; i < BAR_WIDTH; i++) {
        if (i < bar_length)
            printf("█");
        else
            printf(" ");
    }
    printf(" | RMS: %5.3f V | dBFS: %6.1f dB\n", rms, dbfs);
}