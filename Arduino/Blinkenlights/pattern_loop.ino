//byte pattern[] = {1, 2, 4, 8, 16, 32, 64, 128, 64, 32, 16, 8, 4, 2, 129, 66, 36, 24, 36, 66, 129, -1};
//int pattern[] = {0, 0, 0, 0, 0, 0, 0, 0, -1};

int light_pins[8];
int tempo_pin;

int default_pattern[] = {129, 66, 36, 24, 24, 36, 66, 129, -1};
int default_speed = 500;

#define MAX_PATTERN_SIZE 128

int current_pattern[MAX_PATTERN_SIZE];
int current_position;
unsigned long last_frame_millis;
unsigned long millis_per_frame = 500;

void reset_pattern() {
  load_pattern(default_pattern);
  millis_per_frame = default_speed;
}

void toggle_pattern_bit(int i) {
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
