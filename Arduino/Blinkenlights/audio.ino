#define FFT_N 128
#define OCTAVE 1
#define LOG_OUT 1
#include <FFT.h>

#define FFT_BUCKETS (FFT_N / 2)
#define FFT_LIGHTS 8

int audio_pin = 1;
int audio_vcc_pin = 2;
int audio_sensitivity_pin = 3;
int audio_reference_level = 1023;
int audio_level_for_pattern = 120;
int audio_channel_levels[FFT_LIGHTS];

void audio_set_audio_pin(int pin) {
  audio_pin = pin;
}
void audio_set_audio_reference_pin(int pin) {
  audio_vcc_pin = pin;
  audio_reference_level = analogRead(pin);
}
void audio_set_audio_sensitivity_pin(int pin) {
  audio_sensitivity_pin = pin;
}

int audio_data_count = 0;
int audio_pattern = 0;

void audio_setup() {
}

int audio_each_loop(unsigned long now) {
  int i, j;
  int max = audio_reference_level > 0 ? audio_reference_level : analogRead(audio_vcc_pin);
  int sample10 = analogRead(audio_pin);
 
  int sample16 = (sample10 - 512) * 16;

  fft_input[audio_data_count * 2] = sample16;
  fft_input[audio_data_count * 2 + 1] = 0;
  
  audio_data_count++;

  if (audio_data_count >= FFT_N) {
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_octave(); // take the output of the fft

    Serial.print("FFT ");
    Serial.print(": ");
    
    audio_pattern = 0;
    for(i = 0; i < FFT_LIGHTS; i++) {
      Serial.print(fft_oct_out[i], DEC);
      Serial.print(" . ");
      audio_channel_levels[i] = fft_oct_out[i];
      
      if (fft_oct_out[i] > audio_level_for_pattern) {
        audio_pattern = audio_pattern | (1 << i);
      }
    }

    Serial.print(" : ");
    Serial.print(audio_pattern);
    Serial.println();

    audio_data_count = 0;
  }
  else {
    audio_pattern = -1;
  }
}

void audio_active_loop(unsigned long now) {
  if (audio_pattern >= 0) {
    lights_simple_levels(audio_pattern);
    audio_pattern = -1;
  }
}

void audio_on_activation(unsigned long now) {
  lights_set_decay(0);
}

