#include "buttons.h"

// Momentary push buttons — debounced press edge only (Start, Stop)
constexpr uint8_t MOMENTARY_BTN_PINS[] = {BTN_START, BTN_STOP};
constexpr uint8_t MOMENTARY_BTN_IDS[] = {
    static_cast<uint8_t>(ButtonId::Start),
    static_cast<uint8_t>(ButtonId::Stop),
};
constexpr uint8_t MOMENTARY_BTN_COUNT = 2;

struct ButtonState {
  bool stablePressed;
  bool lastReading;
  uint32_t lastChangeMs;
};

ButtonState momentaryBtnStates[MOMENTARY_BTN_COUNT];

void buttonsInit() {
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_STOP, INPUT_PULLUP);
  pinMode(BTN_GEMUK, INPUT_PULLUP);
  pinMode(BTN_KURUS, INPUT_PULLUP);

  for (uint8_t i = 0; i < MOMENTARY_BTN_COUNT; i++) {
    momentaryBtnStates[i].stablePressed = false;
    momentaryBtnStates[i].lastReading = true;
    momentaryBtnStates[i].lastChangeMs = 0;
  }
}

bool isStopPressed() {
  return digitalRead(BTN_STOP) == LOW;
}

bool isGemukSelected() {
  return digitalRead(BTN_GEMUK) == LOW;
}

bool isKurusSelected() {
  return digitalRead(BTN_KURUS) == LOW;
}

ButtonEvent pollButtons() {
  const uint32_t now = millis();
  ButtonEvent event = ButtonEvent::None;

  for (uint8_t i = 0; i < MOMENTARY_BTN_COUNT; i++) {
    const bool reading = digitalRead(MOMENTARY_BTN_PINS[i]) == LOW;

    if (reading != momentaryBtnStates[i].lastReading) {
      momentaryBtnStates[i].lastChangeMs = now;
      momentaryBtnStates[i].lastReading = reading;
    }

    if ((now - momentaryBtnStates[i].lastChangeMs) >= BTN_DEBOUNCE_MS &&
        reading != momentaryBtnStates[i].stablePressed) {
      momentaryBtnStates[i].stablePressed = reading;

      if (reading) {
        switch (static_cast<ButtonId>(MOMENTARY_BTN_IDS[i])) {
          case ButtonId::Start:
            event = ButtonEvent::Start;
            break;
          case ButtonId::Stop:
            event = ButtonEvent::Stop;
            break;
          default:
            break;
        }
      }
    }
  }

  return event;
}
