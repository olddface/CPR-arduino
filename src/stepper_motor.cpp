#include "stepper_motor.h"

#include "buttons.h"
#include "config.h"

void stepperInit() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(STEP_PIN, TB6600_COMMON_5V ? HIGH : LOW);
  digitalWrite(DIR_PIN, TB6600_COMMON_5V ? HIGH : LOW);
}

unsigned long compressionStepDelayUs() {
  const unsigned long compressionPeriodUs =
      60000000UL / static_cast<unsigned long>(COMPRESSIONS_PER_MIN);
  const unsigned long stepsPerCompression = 2UL * STEPS_PER_STROKE;
  return compressionPeriodUs / stepsPerCompression;
}

bool stepMotorTimed(uint16_t steps, bool directionUp, unsigned long delayUs) {
  if (TB6600_COMMON_5V) {
    digitalWrite(DIR_PIN, directionUp ? LOW : HIGH);
    digitalWrite(STEP_PIN, HIGH);
  } else {
    digitalWrite(DIR_PIN, directionUp ? HIGH : LOW);
    digitalWrite(STEP_PIN, LOW);
  }

  for (uint16_t i = 0; i < steps; i++) {
    if (isStopPressed()) {
      return false;
    }

    if (TB6600_COMMON_5V) {
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(5);
      digitalWrite(STEP_PIN, HIGH);
    } else {
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(5);
      digitalWrite(STEP_PIN, LOW);
    }

    if (delayUs > 5) {
      delayMicroseconds(delayUs - 5);
    }
  }

  return true;
}
