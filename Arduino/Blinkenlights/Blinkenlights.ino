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

  last_micros = micros();
//  Serial.print("Loop took ");
//  Serial.print(now - last_micros, DEC);
//  Serial.println(" us");
}


