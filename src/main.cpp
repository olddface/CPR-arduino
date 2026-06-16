/*
  Automatic CPR Portable — thesis flowchart (Gambar 3.3)
  MAX30102 heart rate + I2C LCD + TB6600 stepper + buttons + buzzer

  Wiring: (see README.md for full TB6600 / ENA details)
*/

// #include <...>  = standard/system library (angle brackets)
// #include "..."  = our project header from include/ folder
#include <Arduino.h>

#include "buttons.h"
#include "buzzer.h"
#include "cpr_engine.h"
#include "display.h"
#include "load_cell.h"
#include "pulse_sensor.h"
#include "stepper_motor.h"

// void = function returns nothing
// setup() runs ONCE after power-on or reset — Arduino required entry point
void setup() {
  // Start USB serial at 115200 bits per second for debug prints
  Serial.begin(115200);

  // Call init functions in each module (each .cpp file owns one hardware area)
  stepperInit();
  buzzerInit();
  buttonsInit();
  loadCellInit();
  displayInit();

  // F("text") stores string in flash memory (saves RAM on Arduino)
  Serial.println(F("Initializing..."));

  // if (!x) means "if x is false" — pulseSensorInit returns bool (true = OK)
  if (!pulseSensorInit()) {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    showSensorNotFoundDisplay();
    // while (1) = infinite loop — halt forever if sensor missing
    while (1) {
      delay(1000);  // delay(ms) blocks for 1000 milliseconds
    }
  }

  cprEngineInit();
  Serial.println(F("Automatic CPR ready — press Start"));
}

// loop() runs forever after setup() — Arduino required entry point
void loop() {
  cprEngineUpdate();  // one tick of the CPR state machine
}
