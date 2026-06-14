#pragma once

#include <Arduino.h>

// --- Timing ---
constexpr uint32_t UI_UPDATE_MS = 500;
constexpr uint32_t PULSE_CHECK_MS = 2500;
constexpr uint32_t STOP_DISPLAY_MS = 2000;
constexpr uint32_t BTN_DEBOUNCE_MS = 40;
constexpr uint32_t BEEP_ON_MS = 200;
constexpr uint32_t BEEP_OFF_MS = 150;

// --- Buttons & buzzer ---
constexpr uint8_t BTN_START = 2;
constexpr uint8_t BTN_GEMUK = 4;
constexpr uint8_t BTN_STOP = 7;
constexpr uint8_t BTN_KURUS = 12;
constexpr uint8_t BUZZER_PIN = 13;

// --- TB6600 stepper ---
constexpr uint8_t STEP_PIN = 9;
constexpr uint8_t DIR_PIN = 8;
constexpr uint8_t ENABLE_PIN = 10;
constexpr bool TB6600_COMMON_5V = true;
// ENA wiring must match DIR/PUL opto style. Toggle if idle shaft stays stiff (driver still on).
// ENA- → D10, ENA+ → 5V Disabled by default
constexpr bool TB6600_ENABLE_5V = false;

// --- CPR parameters ---
constexpr uint8_t GEMUK_COMPRESSIONS = 30;
constexpr uint8_t KURUS_COMPRESSIONS = 15;
constexpr uint8_t VENTILATION_BEEPS = 2;
constexpr uint16_t COMPRESSIONS_PER_MIN = 110;
constexpr uint16_t STEPS_PER_REV = 200;
constexpr uint16_t STEPS_PER_STROKE = 200;

// --- Pulse detection ---
constexpr long FINGER_IR_MIN = 70000;
constexpr int PULSE_BPM_MIN = 20;

enum class ButtonId : uint8_t { Start, Gemuk, Stop, Kurus };

enum class ButtonEvent : uint8_t { None, Start, Gemuk, Stop, Kurus };

enum class SystemState : uint8_t {
  Idle,
  PulseCheck,
  AwaitingMode,
  RunningGemuk,
  RunningKurus,
};
