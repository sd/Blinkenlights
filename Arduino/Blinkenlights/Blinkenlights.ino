#include <Max3421e.h>

//#include <Usb.h>
//#include <AndroidAccessory.h>
//AndroidAccessory acc("Not So Stupid",
//		     "Blinkenlights",
//		     "Blinkenlights Board",
//		     "1.0",
//		     "http://www.notso.net/",
//		     "0000000020121212");

#include "fix_fft.h"

#define MODE_PATTERN 0
#define MODE_FFT 1
#define TOTAL_MODE_COUNT 2
#define DEFAULT_DECAY 32

int mode = MODE_FFT;

int mode_switch_pin = 52;
int mode_switch_prev_state = 0;

void setup() {                
  Serial.begin(115200);
  Serial.println("\r\nStart");

  randomSeed(analogRead(0));
  
//  setup_light_pins(22, 24, 26, 28, 30, 32, 34, 36);
//  setup_tempo_pin(13);
//  reset_pattern();  

  int light_pins[] = {22, 24, 26, 28, 30, 32, 34, 36};
  setup_lights(light_pins);
  
//  light_levels(0, 1, 2, 3, 4, 5, 6, 7);
//  simple_light_levels(0);
  set_lights_decay(0);
  
  setup_audio_pin(1);
  setup_audio_reference_pin(2);
  setup_audio_sensitivity_pin(3);

  // acc.powerOn();
  
  pinMode(mode_switch_pin, INPUT);
}

unsigned long last_micros = 0;

void loop() {
  int x;

  unsigned long now = micros();

  int mode_switch_state = digitalRead(mode_switch_pin);

  if (mode_switch_state != mode_switch_prev_state) {
    if (mode_switch_state == LOW) {
      mode = (mode + 1) % TOTAL_MODE_COUNT;
      
      switch(mode) { // one-time on mode change
      case MODE_PATTERN:
        set_lights_decay(DEFAULT_DECAY);
        break;
      case MODE_FFT:
        set_lights_decay(0);
        break;
      }
    }
    mode_switch_prev_state = mode_switch_state;
  }

  switch(mode) {
  case MODE_PATTERN:
    lights_program(now);
    break;
  case MODE_FFT:
    x = read_audio_sample();
    if (x >= 0)
      simple_light_levels(x);
    break;
  }

  lights_loop(now);

//  process_usb_command();

  last_micros = micros();
//  Serial.print("Loop took ");
//  Serial.print(now - last_micros, DEC);
//  Serial.println(" us");
}

#define COMMAND_RESET 0
#define COMMAND_PROGRAM_CHANNEL 1
#define COMMAND_FASTER 2
#define COMMAND_SLOWER 3
#define COMMAND_MIC_DATA 99



//void process_usb_command() {
//  int i, j;
//  byte msg[260];
//  
//  if (acc.isConnected()) {
//    int len = acc.read(msg, sizeof(msg), 1);
//    if (len > 0) {
//      byte command = msg[0];
//      byte target = msg[1];
//      byte data = msg[2];
//
//      switch (command) {
//      case COMMAND_RESET:
//        reset_pattern();
//        break;
//      case COMMAND_PROGRAM_CHANNEL:
//        toggle_pattern_bit(target);
//        apply_current_pattern();
//        break;
//      case COMMAND_FASTER:
//        increase_pattern_speed();
//        break;
//      case COMMAND_SLOWER:
//        decrease_pattern_speed();
//        break;
//      case COMMAND_MIC_DATA:
////        Serial.print("Data ");
////        Serial.print(msg[1]);
////        Serial.print(" = ");
////        Serial.print((char)msg[2], DEC);
////        Serial.print(".");
////        Serial.print((char)msg[3], DEC);
////        Serial.print(".");
////        Serial.print((char)msg[4], DEC);
////        Serial.print(".");
////        Serial.print((char)msg[5], DEC);
////        Serial.print(".");
////        Serial.print((char)msg[6], DEC);
////        Serial.print(".");
////        Serial.print((char)msg[7], DEC);
////        Serial.print(".");
////        Serial.print((char)msg[8], DEC);
////        Serial.print(".");
////        Serial.print((char)msg[9], DEC);
////        Serial.print(".");
////        Serial.print((char)msg[10], DEC);
////        Serial.println();
//
////        int dataLength = msg[1] + 1;
////        
////        for (i = 0; (i < dataLength) && (audio_data_count < FFT_SAMPLES); i++, audio_data_count++) {
////          audio_data[audio_data_count] = msg[2 + i];
////          audio_data_im[i] = 0;
////        }
////        
////        if (audio_data_count >= FFT_SAMPLES) {
////          fix_fft(audio_data, audio_data_im, FFT_N, 0);
////          
////          Serial.print("FFT ");
////          Serial.print(": ");
////          
////          for (i = 0; i < (FFT_SAMPLES / 2); i++) {
////            audio_data[i] = sqrt(audio_data[i] * audio_data[i] + audio_data_im[i] * audio_data_im[i]); 
////            Serial.print(audio_data[i], DEC);
////            Serial.print(".");
////          }
////          
////          Serial.print(" / ");
////          byte pattern = 0;
////          int sum = 0;
////          for(i = 0; i < 8; i++) {
////            sum = 0;
////            for (j = 0; j < (FFT_SAMPLES / 16); j++) {
////              sum += 1 << audio_data[(i * 8) + j];
////            }
////            Serial.print(sum, DEC);
////            Serial.print(".");
////            if (sum > 32) {
////               pattern = pattern | (1 << i);
////            }
////          }
////            
////          Serial.print("\n");
////
////          apply_light_pattern(pattern);
////          last_frame_millis = millis();
////          audio_data_count = 0;
////        }
//        break;
//      }
//    }
//  }
//}



