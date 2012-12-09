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
  
  int light_pins[] = {22, 24, 26, 28, 30, 32, 34, 36};
  lights_setup(light_pins);
  
//  light_levels(0, 1, 2, 3, 4, 5, 6, 7);
//  simple_light_levels(0);
  lights_set_decay(0);
  
  audio_setup();
  audio_set_audio_pin(1);
  audio_set_audio_reference_pin(2);
  audio_set_audio_sensitivity_pin(3);

  matrix_setup(2, 3, 4);

  pinMode(mode_switch_pin, INPUT);
  
  process_mode_activation(micros());
}

unsigned long last_micros = 0;

void loop() {
  int x;

  unsigned long now = micros();

  int mode_switch_state = digitalRead(mode_switch_pin);

  if (mode_switch_state != mode_switch_prev_state) {
    if (mode_switch_state == LOW) {
      mode = (mode + 1) % TOTAL_MODE_COUNT;
      process_mode_activation(now);
    }
    mode_switch_prev_state = mode_switch_state;
  }

  audio_each_loop(now);
  lights_each_loop(now);
  pattern_each_loop(now);
  matrix_each_loop(now);

  switch(mode) {
  case MODE_PATTERN:
    pattern_active_loop(now);
    break;
  case MODE_FFT:
    audio_active_loop(now);
    break;
  }

  last_micros = micros();
//  Serial.print("Loop took ");
//  Serial.print(now - last_micros, DEC);
//  Serial.println(" us");
}

void process_mode_activation(unsigned long now) {
  switch(mode) { // one-time on mode change
  case MODE_PATTERN:
    pattern_on_activation(now);
    break;
  case MODE_FFT:
    audio_on_activation(now);
    break;
  }
}


