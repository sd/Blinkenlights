#include <Max3421e.h>
#include <Usb.h>
#include <AndroidAccessory.h>

//byte pattern[] = {1, 2, 4, 8, 16, 32, 64, 128, 64, 32, 16, 8, 4, 2, 129, 66, 36, 24, 36, 66, 129, -1};
//int pattern[] = {0, 0, 0, 0, 0, 0, 0, 0, -1};
int default_pattern[] = {129, 66, 36, 24, 24, 36, 66, 129, -1};
int default_speed = 200;

int light_pins[8];
int tempo_pin;

AndroidAccessory acc("Not So Stupid",
		     "Blinkenlights",
		     "Blinkenlights Board",
		     "1.0",
		     "http://www.notso.net/",
		     "0000000020121212");

// the setup routine runs once when you press reset:
void setup() {                
  Serial.begin(115200);
  Serial.println("\r\nStart");

//  setup_light_pins(2, 3, 4, 5, 6, 7, 8, 9);
  setup_light_pins(22, 24, 26, 28, 30, 32, 34, 36);
  setup_tempo_pin(13);
  
  reset_all();
  
  acc.powerOn();
}

// the loop routine runs over and over again forever:
void loop() {
  process_usb_command();
  
  pattern_loop();

  delay(10);
}

#define COMMAND_RESET 0
#define COMMAND_PROGRAM_CHANNEL 1
#define COMMAND_FASTER 2
#define COMMAND_SLOWER 3

#define MAX_PATTERN_SIZE 128

int current_pattern[MAX_PATTERN_SIZE];
int current_position;
unsigned long last_frame_millis;
unsigned long millis_per_frame = 500;


void process_usb_command() {
  byte msg[3];
  
  if (acc.isConnected()) {
    int len = acc.read(msg, sizeof(msg), 1);
    if (len > 0) {
      byte command = msg[0];
      byte target = msg[1];
      byte data = msg[2];

      Serial.print("-- Command ");
      Serial.print(command);
      Serial.print(" target ");
      Serial.print(target);
      Serial.println("");

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
