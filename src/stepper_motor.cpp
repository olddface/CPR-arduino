#include "stepper_motor.h"

#include "buttons.h"
#include "config.h"

// namespace { } = file-private scope; names inside are not visible to other .cpp files
namespace {

// Ternary: condition ? valueIfTrue : valueIfFalse
constexpr uint8_t kEnableActiveLevel = TB6600_ENABLE_5V ? LOW : HIGH;
constexpr uint8_t kEnableInactiveLevel = TB6600_ENABLE_5V ? HIGH : LOW;

void stepperSetEnabled(bool enabled) {
  // digitalWrite(pin, HIGH or LOW) sets output voltage on that pin
  digitalWrite(ENABLE_PIN, enabled ? kEnableActiveLevel : kEnableInactiveLevel);
}

}  // namespace

void stepperInit() {
  // pinMode(pin, OUTPUT) makes pin a digital output (vs INPUT / INPUT_PULLUP)
  pinMode(ENABLE_PIN, OUTPUT);
  stepperSetEnabled(false);  // motor driver off at boot
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(STEP_PIN, TB6600_COMMON_5V ? HIGH : LOW);
  digitalWrite(DIR_PIN, TB6600_COMMON_5V ? HIGH : LOW);
}

// unsigned long = 0..4,294,967,295, used here for microsecond timing
unsigned long compressionStepDelayUs() {
  // const = this variable cannot be reassigned inside the function
  const unsigned long compressionPeriodUs =
      60000000UL / static_cast<unsigned long>(COMPRESSIONS_PER_MIN);
  // UL suffix = unsigned long literal
  const unsigned long stepsPerCompression = 2UL * STEPS_PER_STROKE;
  return compressionPeriodUs / stepsPerCompression;
}

// bool return: true = finished all steps, false = stopped early (Stop button)
// Parameters: steps count, directionUp flag, delay between steps in microseconds
bool stepMotorTimed(uint16_t steps, bool directionUp, unsigned long delayUs) {
  stepperSetEnabled(true);  // energize motor only for this stroke

  if (TB6600_COMMON_5V) {
    digitalWrite(DIR_PIN, directionUp ? LOW : HIGH);
    digitalWrite(STEP_PIN, HIGH);
  } else {
    digitalWrite(DIR_PIN, directionUp ? HIGH : LOW);
    digitalWrite(STEP_PIN, LOW);
  }

  // for (init; condition; increment) — classic counted loop
  for (uint16_t i = 0; i < steps; i++) {
    if (isStopPressed()) {
      stepperSetEnabled(false);
      return false;  // exit function immediately
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
