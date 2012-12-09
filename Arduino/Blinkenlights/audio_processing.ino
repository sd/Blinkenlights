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
int level_for_pattern = (80 * (FFT_BUCKETS / FFT_LIGHTS));

void setup_audio_pin(int pin) {
  audio_pin = pin;
}
void setup_audio_reference_pin(int pin) {
  audio_vcc_pin = pin;
  audio_reference_level = analogRead(pin);
}
void setup_audio_sensitivity_pin(int pin) {
  audio_sensitivity_pin = pin;
}

int audio_data_count = 0;

int read_audio_sample() {
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
          
    int pattern = 0;
    Serial.print(" / ");
    for(i = 0; i < FFT_LIGHTS; i++) {
      Serial.print(fft_oct_out[i], DEC);
      Serial.print(" . ");
      if (fft_oct_out[i] > 90) {
        pattern = pattern | (1 << i);
      }
    }

    Serial.print(" : ");
    Serial.print(pattern);
    Serial.println();

    audio_data_count = 0;
    return pattern;
  }
  return -1;
}


