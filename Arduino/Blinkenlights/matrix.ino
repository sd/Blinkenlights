#include "HT1632.h"

HT1632LEDMatrix * matrix;

unsigned long matrix_last_micros = 0;

void matrix_setup(int data_pin, int write_pin, int select_pin) {
  matrix = new HT1632LEDMatrix(data_pin, write_pin, select_pin);
  matrix->begin(HT1632_COMMON_16NMOS);
  Serial.println("Clear Matrix");
  matrix->clearScreen(); 
  Serial.print("Matrix ");
  Serial.print(matrix->width());
  Serial.print(" x ");
  Serial.print(matrix->height());
  Serial.println("");
}

void matrix_on_activation(unsigned long now) {
}

void matrix_each_loop(unsigned long now) {
  if (now < matrix_last_micros) {
    // we looped the counter, let's start again
    matrix_last_micros = now;
  }

  if (now - matrix_last_micros < (100000)) {
    return;
  }

  matrix->setPixel(random(0, matrix->width()), random(0, matrix->height()));
  matrix->clrPixel(random(0, matrix->width()), random(0, matrix->height()));
  matrix->writeScreen();

  matrix_last_micros = now;
}

void matrix_active_loop(unsigned long now) {
}


