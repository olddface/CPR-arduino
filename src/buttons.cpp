#include "buttons.h"

constexpr uint8_t BTN_PINS[] = {BTN_START, BTN_GEMUK, BTN_STOP, BTN_KURUS};
constexpr uint8_t BTN_COUNT = 4;

struct ButtonState {
  bool stablePressed;
  bool lastReading;
  uint32_t lastChangeMs;
};

ButtonState btnStates[BTN_COUNT];

void buttonsInit() {
  for (uint8_t i = 0; i < BTN_COUNT; i++) {
    pinMode(BTN_PINS[i], INPUT_PULLUP);
    btnStates[i].stablePressed = false;
    btnStates[i].lastReading = true;
    btnStates[i].lastChangeMs = 0;
  }
}

bool isStopPressed() {
  return digitalRead(BTN_STOP) == LOW;
}

ButtonEvent pollButtons() {
  const uint32_t now = millis();
  ButtonEvent event = ButtonEvent::None;

  for (uint8_t i = 0; i < BTN_COUNT; i++) {
    const bool reading = digitalRead(BTN_PINS[i]) == LOW;

    if (reading != btnStates[i].lastReading) {
      btnStates[i].lastChangeMs = now;
      btnStates[i].lastReading = reading;
    }

    if ((now - btnStates[i].lastChangeMs) >= BTN_DEBOUNCE_MS &&
        reading != btnStates[i].stablePressed) {
      btnStates[i].stablePressed = reading;

      if (reading) {
        switch (static_cast<ButtonId>(i)) {
          case ButtonId::Start:
            event = ButtonEvent::Start;
            break;
          case ButtonId::Gemuk:
            event = ButtonEvent::Gemuk;
            break;
          case ButtonId::Stop:
            event = ButtonEvent::Stop;
            break;
          case ButtonId::Kurus:
            event = ButtonEvent::Kurus;
            break;
        }
      }
    }
  }

  return event;
}
