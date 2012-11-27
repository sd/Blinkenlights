#include <Max3421e.h>
#include <Usb.h>
#include <AndroidAccessory.h>
AndroidAccessory acc("Not So Stupid",
		     "Blinkenlights",
		     "Blinkenlights Board",
		     "1.0",
		     "http://www.notso.net/",
		     "0000000020121212");

#include "fix_fft.h"

//byte pattern[] = {1, 2, 4, 8, 16, 32, 64, 128, 64, 32, 16, 8, 4, 2, 129, 66, 36, 24, 36, 66, 129, -1};
//int pattern[] = {0, 0, 0, 0, 0, 0, 0, 0, -1};
int default_pattern[] = {129, 66, 36, 24, 24, 36, 66, 129, -1};
int default_speed = 500;

int light_pins[8];
int tempo_pin;

// the setup routine runs once when you press reset:
void setup() {                
  Serial.begin(115200);
  Serial.println("\r\nStart");

//  setup_light_pins(2, 3, 4, 5, 6, 7, 8, 9);
  setup_light_pins(22, 24, 26, 28, 30, 32, 34, 36);
  setup_tempo_pin(13);
  
  setup_audio_pins(1, 2);
  reset_all();
  
  acc.powerOn();
}

// the loop routine runs over and over again forever:
void loop() {
//  process_usb_command();
  
//  pattern_loop();

  read_audio_sample();
}

#define COMMAND_RESET 0
#define COMMAND_PROGRAM_CHANNEL 1
#define COMMAND_FASTER 2
#define COMMAND_SLOWER 3
#define COMMAND_MIC_DATA 99

#define MAX_PATTERN_SIZE 128

int current_pattern[MAX_PATTERN_SIZE];
int current_position;
unsigned long last_frame_millis;
unsigned long millis_per_frame = 500;

#define FFT_SAMPLES 128
#define FFT_N 7

int audio_pin = 1;
int audio_vcc_pin = 2;
void setup_audio_pins(int pin, int vcc_pin) {
  audio_pin = pin;
  audio_vcc_pin = vcc_pin;
}

#define THREE_VOLTS = 690;  // math says 375, but measured it directly it gives 690

char audio_data[FFT_SAMPLES];
char audio_data_im[FFT_SAMPLES];
int audio_data_count = 0;

char read_audio_sample() {
  int i, j;
  int max = analogRead(audio_vcc_pin);
  int sample10 = analogRead(audio_pin);
  int sample10zero = (sample10 - (max / 2));
  char sample8 = sample10zero * 127 / max;

  audio_data[audio_data_count] = sample8;
  audio_data_im[audio_data_count] = 0;
  audio_data_count++;

  if (audio_data_count >= FFT_SAMPLES) {
    fix_fft(audio_data, audio_data_im, FFT_N, 0);
          
    Serial.print("FFT ");
    Serial.print(": ");
          
    for (i = 0; i < (FFT_SAMPLES / 2); i++) {
      audio_data[i] = sqrt(audio_data[i] * audio_data[i] + audio_data_im[i] * audio_data_im[i]); 
      Serial.print(audio_data[i], DEC);
      Serial.print(".");
    }
          
    Serial.print(" / ");
    byte pattern = 0;
    int sum = 0;
    for(i = 0; i < 8; i++) {
      sum = 0;
      for (j = 0; j < (FFT_SAMPLES / 16); j++) {
        sum += audio_data[(i * 8) + j]; // 1 << audio_data[(i * 8) + j];
      }
      Serial.print(sum, DEC);
      Serial.print(".");
      if (sum > 15) {
         pattern = pattern | (1 << i);
      }
    }
      
    Serial.print("\n");

    apply_light_pattern(pattern);
    audio_data_count = 0;
  }
}

void process_usb_command() {
  int i, j;
  byte msg[260];
  
  if (acc.isConnected()) {
    int len = acc.read(msg, sizeof(msg), 1);
    if (len > 0) {
      byte command = msg[0];
      byte target = msg[1];
      byte data = msg[2];

      switch (command) {
      case COMMAND_RESET:
        reset_all();
        break;
      case COMMAND_PROGRAM_CHANNEL:
        toggle_pattern(target);
        apply_light_pattern(current_pattern[current_position]);
        break;
      case COMMAND_FASTER:
        millis_per_frame = 2 * millis_per_frame / 3;
        Serial.print(" - speed ");
        Serial.print(millis_per_frame);
        Serial.println("");
        break;
      case COMMAND_SLOWER:
        millis_per_frame = 4 * millis_per_frame / 3;
        Serial.print(" - speed ");
        Serial.print(millis_per_frame);
        Serial.println(""); 
        break;
      case COMMAND_MIC_DATA:
//        Serial.print("Data ");
//        Serial.print(msg[1]);
//        Serial.print(" = ");
//        Serial.print((char)msg[2], DEC);
//        Serial.print(".");
//        Serial.print((char)msg[3], DEC);
//        Serial.print(".");
//        Serial.print((char)msg[4], DEC);
//        Serial.print(".");
//        Serial.print((char)msg[5], DEC);
//        Serial.print(".");
//        Serial.print((char)msg[6], DEC);
//        Serial.print(".");
//        Serial.print((char)msg[7], DEC);
//        Serial.print(".");
//        Serial.print((char)msg[8], DEC);
//        Serial.print(".");
//        Serial.print((char)msg[9], DEC);
//        Serial.print(".");
//        Serial.print((char)msg[10], DEC);
//        Serial.println();
        int dataLength = msg[1] + 1;
        
        for (i = 0; (i < dataLength) && (audio_data_count < FFT_SAMPLES); i++, audio_data_count++) {
          audio_data[audio_data_count] = msg[2 + i];
          audio_data_im[i] = 0;
        }
        
        if (audio_data_count >= FFT_SAMPLES) {
          fix_fft(audio_data, audio_data_im, FFT_N, 0);
          
          Serial.print("FFT ");
          Serial.print(": ");
          
          for (i = 0; i < (FFT_SAMPLES / 2); i++) {
            audio_data[i] = sqrt(audio_data[i] * audio_data[i] + audio_data_im[i] * audio_data_im[i]); 
            Serial.print(audio_data[i], DEC);
            Serial.print(".");
          }
          
          Serial.print(" / ");
          byte pattern = 0;
          int sum = 0;
          for(i = 0; i < 8; i++) {
            sum = 0;
            for (j = 0; j < (FFT_SAMPLES / 16); j++) {
              sum += 1 << audio_data[(i * 8) + j];
            }
            Serial.print(sum, DEC);
            Serial.print(".");
            if (sum > 32) {
               pattern = pattern | (1 << i);
            }
          }
            
          Serial.print("\n");

          apply_light_pattern(pattern);
          last_frame_millis = millis();
          audio_data_count = 0;
        }
        break;
      }
    }
  }
}

void reset_all() {
  load_pattern(default_pattern);
  millis_per_frame = default_speed;
}

void toggle_pattern(int i) {
  Serial.print("  - Toggle ");
  Serial.print(current_pattern[current_position]);
  if (current_pattern[current_position] >= 0) {
    current_pattern[current_position] = current_pattern[current_position] ^ (0x01 << i);
  }
  Serial.print(" to ");
  Serial.println(current_pattern[current_position]);
}

void setup_light_pins(int pin0, int pin1, int pin2, int pin3, int pin4, int pin5, int pin6, int pin7) {
  light_pins[0] = pin0;
  light_pins[1] = pin1;
  light_pins[2] = pin2;
  light_pins[3] = pin3;
  light_pins[4] = pin4;
  light_pins[5] = pin5;
  light_pins[6] = pin6;
  light_pins[7] = pin7;
  for(int i = 0; i < 8; i++) {
    digitalWrite(light_pins[i], LOW);
    pinMode(light_pins[i], OUTPUT);     
    digitalWrite(light_pins[i], LOW);
  }
}

void setup_tempo_pin(int pin) {
  tempo_pin = pin;
  pinMode(pin, OUTPUT);
  digitalWrite(tempo_pin, LOW);
}

void apply_light_pattern(int pattern) {  
  for(int i = 0; i < 8; i++) {
    digitalWrite(light_pins[i], (pattern & (1 << i)) == 0 ? LOW : HIGH);
  }
}

void load_pattern(int * pattern) {
  for(int i = 0; i < MAX_PATTERN_SIZE; i++) {
    current_pattern[i] = pattern[i];
    if (current_pattern[i] == -1)
      break;
  }
  unsigned long  current_millis = millis();
  current_position = 0;
  last_frame_millis = 0;
}

void set_speet(int speed) {
  millis_per_frame = speed;
}

void set_frames_per_second(int frames_per_second) {
  millis_per_frame = 1000 / frames_per_second;
}

void pattern_loop() {
  unsigned long current_millis = millis();
  if (last_frame_millis + millis_per_frame < current_millis) {
    last_frame_millis = current_millis;

    current_position++;
    if (current_pattern[current_position] == -1) {
      current_position = 0;
    }

    apply_light_pattern(current_pattern[current_position]);
    
    if (current_position % 2 == 0) {
      digitalWrite(tempo_pin, HIGH);
    }
    else {
      digitalWrite(tempo_pin, LOW);
    }
  }
}
