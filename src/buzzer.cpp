#include "buzzer.h"

void buzzerInit() {
  pinMode(BUZZER_PIN, OUTPUT);
  buzzerOff();
}

void buzzerOn() {
  digitalWrite(BUZZER_PIN, HIGH);  // active buzzer: HIGH = sound on
}

void buzzerOff() {
  digitalWrite(BUZZER_PIN, LOW);
}

// count = how many beeps; onMs/offMs = timing (defaults set in buzzer.h)
void buzzerBeep(uint8_t count, uint32_t onMs, uint32_t offMs) {
  for (uint8_t i = 0; i < count; i++) {
    buzzerOn();
    delay(onMs);
    buzzerOff();
    // Skip pause after the last beep (i+1 < count avoids delay after final beep)
    if (i + 1 < count) {
      delay(offMs);
    }
  }
}
