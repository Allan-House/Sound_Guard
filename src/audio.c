#include "audio.h"
#include "config.h"
#include <stdio.h>
#include <math.h>

float audio_normalize_rms(float rms) {
    // Pré-calculando constantes para otimização
    const float rms_scale = 1.0f / MAX_RMS;
    
    // Normaliza o sinal com base no RMS de uma senóide de pico 1V (0 = silencioso, 1 = máximo) e a limita
    float normalized = rms * rms_scale;
    if (normalized > 1.0f) normalized = 1.0f;
    return normalized;
}

float audio_calculate_dbfs(float rms) {
    // Pré-calculando constantes para otimização
    const float rms_scale = 1.0f / MAX_RMS;
    const float log_scale = 20.0f / log(10.0f);
    
    // Cálculo do valor dBFS do sinal RMS
    return log_scale * logf((rms > 0.0001f ? rms : 0.0001f) * rms_scale);
}

int audio_calculate_bar_length(float normalized) {
    float norm_clamped = (normalized < MIN_NORMALIZED) ? MIN_NORMALIZED : normalized;
    int barLength = (int)((logf(norm_clamped) - logf(MIN_NORMALIZED)) / 
                          (-logf(MIN_NORMALIZED)) * BAR_WIDTH);
    if (barLength < 0) barLength = 0;
    if (barLength > BAR_WIDTH) barLength = BAR_WIDTH;
    return barLength;
}

void audio_print_bar(float rms, float dbfs) {
    float normalized = audio_normalize_rms(rms);
    int barLength = audio_calculate_bar_length(normalized);
    
    printf("Volume: ");
    for (int i = 0; i < BAR_WIDTH; i++) {
        if (i < barLength)
            printf("█");
        else
            printf(" ");
    }
    printf(" | RMS: %5.3f V | dBFS: %6.1f dB\n", rms, dbfs);
}