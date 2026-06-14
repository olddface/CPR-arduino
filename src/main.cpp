/*
  Automatic CPR Portable — thesis flowchart (Gambar 3.3)
  MAX30102 heart rate + I2C LCD + TB6600 stepper + buttons + buzzer

  Wiring:
    MAX30102: 5V, GND, SDA=A4, SCL=A5
    LCD I2C:  same SDA/SCL bus, addr 0x27 (or 0x3F)
    Buttons (INPUT_PULLUP, pressed = LOW):
      D2  = Start
      D4  = Gemuk mode
      D7  = Stop
      D12 = Kurus mode
    Buzzer (active): D13 → buzzer+, GND → buzzer-
    TB6600 (optocoupler inputs, common GND):
      PUL+  → D9,  DIR+  → D8,  ENA+  → D10
      PUL-, DIR-, and ENA- jumpered together → Arduino GND
      (if PUL-/DIR- go to GPIO and PUL+/DIR+ to 5V instead, set TB6600_COMMON_5V to true)
      ENA off at idle; enabled only during stepMotorTimed() strokes
      (toggle TB6600_ENABLE_5V if idle shaft stays stiff)
      Motor: A+/A- and B+/B- to stepper coils
      Driver GND → Arduino GND; motor PSU 9–42 V on VCC/GND (NOT Arduino 5V)
      Set TB6600 microsteps to match STEPS_PER_REV in config.h
*/

#include <Arduino.h>

#include "buttons.h"
#include "buzzer.h"
#include "cpr_engine.h"
#include "display.h"
#include "pulse_sensor.h"
#include "stepper_motor.h"

void setup() {
  Serial.begin(115200);

  stepperInit();
  buzzerInit();
  buttonsInit();
  displayInit();

  Serial.println(F("Initializing..."));

  if (!pulseSensorInit()) {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    showSensorNotFoundDisplay();
    while (1) {
      delay(1000);
    }
  }

  cprEngineInit();
  Serial.println(F("Automatic CPR ready — press Start"));
}

void loop() {
  cprEngineUpdate();
}
