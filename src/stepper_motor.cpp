#include "stepper_motor.h"

#include "buttons.h"
#include "config.h"

namespace {

constexpr uint8_t kEnableActiveLevel = TB6600_ENABLE_5V ? LOW : HIGH;
constexpr uint8_t kEnableInactiveLevel = TB6600_ENABLE_5V ? HIGH : LOW;

void stepperSetEnabled(bool enabled) {
  digitalWrite(ENABLE_PIN, enabled ? kEnableActiveLevel : kEnableInactiveLevel);
}

}  // namespace

void stepperInit() {
  pinMode(ENABLE_PIN, OUTPUT);
  stepperSetEnabled(false);
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
  stepperSetEnabled(true);

  if (TB6600_COMMON_5V) {
    digitalWrite(DIR_PIN, directionUp ? LOW : HIGH);
    digitalWrite(STEP_PIN, HIGH);
  } else {
    digitalWrite(DIR_PIN, directionUp ? HIGH : LOW);
    digitalWrite(STEP_PIN, LOW);
  }

  for (uint16_t i = 0; i < steps; i++) {
    if (isStopPressed()) {
      stepperSetEnabled(false);
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

  stepperSetEnabled(false);
  return true;
}
