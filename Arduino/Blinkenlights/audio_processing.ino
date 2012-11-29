#define FFT_SAMPLES 128
#define FFT_N 7

int audio_pin = 1;
int audio_vcc_pin = 2;
int audio_sensitivity_pin = 3;
int audio_reference_level = 675;
int level_for_pattern = 8;

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

char audio_data[FFT_SAMPLES];
char audio_data_im[FFT_SAMPLES];
int audio_data_count = 0;

int read_audio_sample() {
  int i, j;
  int max = audio_reference_level > 0 ? audio_reference_level : analogRead(audio_vcc_pin);
  int sample10 = analogRead(audio_pin);
  
  int level = (analogRead(audio_sensitivity_pin) >> 7) + 1; // 1-8
  sample10 = sample10 * level;
  
  int sample10zero = (sample10 - (max / 2));
  char sample8 = sample10zero * 127 / max;

  audio_data[audio_data_count] = sample8;
  audio_data_im[audio_data_count] = 0;
  audio_data_count++;

  if (audio_data_count >= FFT_SAMPLES) {
    fix_fft(audio_data, audio_data_im, FFT_N, 0);
          
Serial.print(level, DEC);
Serial.print(" ");
    Serial.print("FFT ");
    Serial.print(": ");
          
    for (i = 0; i < (FFT_SAMPLES / 2); i++) {
      audio_data[i] = sqrt(audio_data[i] * audio_data[i] + audio_data_im[i] * audio_data_im[i]); 
      Serial.print(audio_data[i], DEC);
      Serial.print(".");
    }
          
    Serial.print(" / ");
    int pattern = 0;
    int sum = 0;
    for(i = 0; i < 8; i++) {
      sum = 0;
      for (j = 0; j < (FFT_SAMPLES / 16); j++) {
        sum += audio_data[(i * 8) + j]; // 1 << audio_data[(i * 8) + j];
      }
      Serial.print(sum, DEC);
      Serial.print(".");
      if (sum > level_for_pattern) {
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

