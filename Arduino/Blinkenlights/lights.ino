#define LIGHTS 8
#define DEFAULT_SPEED 10
#define SHADES_OF_LIGHT 8

int light_pins[8];
char lights[LIGHTS];

int light_loop_speed = DEFAULT_SPEED;
unsigned long lights_last_loop_micros = 0;
int light_loop_counter = 0;

int lights_decay_rate = 0;
int lights_decay_counter = 0;

#define DECAY_STEPS 16
int decay_decrements[] = {0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1}; // 7777665432221111

void lights_setup(int pins[]) {
  int i;
  for (i = 0; i < LIGHTS; i++) {
    light_pins[i] = pins[i];
    lights[i] = 0;
    digitalWrite(light_pins[i], LOW);
    pinMode(light_pins[i], OUTPUT);     
    digitalWrite(light_pins[i], LOW);
  }
}

void light_level(int light, char level) {
  lights[light] = level;
}

void light_levels(char levels[]) {
  for(int i = 0; i < LIGHTS; i++) {
    lights[i] = levels[i];
  }
}

void light_levels(char l0, char l1, char l2, char l3, char l4, char l5, char l6, char l7) {
  lights[0] = l0;
  lights[1] = l1;
  lights[2] = l2;
  lights[3] = l3;
  lights[4] = l4;
  lights[5] = l5;
  lights[6] = l6;
  lights[7] = l7;
}

void lights_simple_levels(int x) {
  for(int i = 0; i < 8; i++) {
    if (x & (1 << i)) {
      lights[i] = SHADES_OF_LIGHT - 1;
    }
    else if (lights_decay_rate == 0) {
      lights[i] = 0;
    }
  }
}

void lights_set_decay(int rate) {
  lights_decay_rate = rate;
}

void lights_on_activation(unsigned long now) {
}

void lights_each_loop(unsigned long now) {
  int i;

  if (now < lights_last_loop_micros) {
    // we looped the counter, let's start again
    lights_last_loop_micros = now;
  }
  
  if (now - lights_last_loop_micros < light_loop_speed)
    return;
  
  for(i = 0; i < LIGHTS; i++) {
    if (lights[i] <= light_loop_counter)
      digitalWrite(light_pins[i], LOW);
    else
      digitalWrite(light_pins[i], HIGH);
  }  

  lights_last_loop_micros = now;
  light_loop_counter = (light_loop_counter + 1) % SHADES_OF_LIGHT;
  
  if (light_loop_counter == 0 && lights_decay_rate > 0) {
    
    lights_decay_counter = (lights_decay_counter + 1) % (lights_decay_rate * DECAY_STEPS);
    
    if (lights_decay_counter % (lights_decay_rate) == 0) {
      int decrement = decay_decrements[lights_decay_counter / DECAY_STEPS];
      decrement = 1;
      if (decrement > 0) {
      
        for(i = 0; i < LIGHTS; i++) {
          if (lights[i] > 0)
            lights[i] = lights[i] - decrement;
        }
      }
    }
  }
}

void lights_active_loop(unsigned long now) {
}


