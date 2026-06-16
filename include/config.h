// #pragma once = include guard: this file is parsed only once per build
#pragma once

// Pull in Arduino types (uint8_t, HIGH, LOW, pinMode, etc.)
#include <Arduino.h>

// --- Timing ---
// constexpr = compile-time constant (cannot change while program runs)
// uint32_t = unsigned 32-bit integer (0 .. 4,294,967,295), good for milliseconds
constexpr uint32_t UI_UPDATE_MS = 500;
constexpr uint32_t PULSE_CHECK_MS = 2500;
constexpr uint32_t STOP_DISPLAY_MS = 2000;
constexpr uint32_t BTN_DEBOUNCE_MS = 40;
constexpr uint32_t BEEP_ON_MS = 200;
constexpr uint32_t BEEP_OFF_MS = 150;

// --- Buttons & buzzer ---
// Momentary push buttons — one action per press (debounced edge)
constexpr uint8_t BTN_START = 2;
constexpr uint8_t BTN_STOP = 7;
// Toggle switches — leave ON to select mode (level read, not edge)
constexpr uint8_t BTN_GEMUK = 4;
constexpr uint8_t BTN_KURUS = 12;
constexpr uint8_t BUZZER_PIN = 13;

// --- TB6600 stepper ---
constexpr uint8_t STEP_PIN = 9;
constexpr uint8_t DIR_PIN = 8;
constexpr uint8_t ENABLE_PIN = 10;
// bool = true or false
constexpr bool TB6600_COMMON_5V = false;  // false = PUL-/DIR- on GND, PUL+/DIR+ on pins
// ENA- → GND, ENA+ → D10: idle D10 LOW (off), stroke D10 HIGH (on)
constexpr bool TB6600_ENABLE_5V = false;

// --- CPR parameters ---
constexpr uint8_t GEMUK_COMPRESSIONS = 30;
constexpr uint8_t KURUS_COMPRESSIONS = 15;
constexpr uint8_t VENTILATION_BEEPS = 2;
// uint16_t = unsigned 16-bit integer (0..65535)
constexpr uint16_t COMPRESSIONS_PER_MIN = 110;
constexpr uint16_t STEPS_PER_REV = 200;
constexpr uint16_t STEPS_PER_STROKE = 200;

// --- HX711 load cell (50 kg) — belt tension gate ---
constexpr uint8_t HX711_DT_PIN = 5;
constexpr uint8_t HX711_SCK_PIN = 6;
// Calibrate: place known mass on cell, set LOAD_CELL_SCALE = (raw - offset) / kg
constexpr float LOAD_CELL_SCALE = -7050.0f;
constexpr float GEMUK_MIN_WEIGHT_KG = 1.0f;
constexpr float KURUS_MIN_WEIGHT_KG = 0.5f;
constexpr unsigned long BELT_TIGHTEN_STEP_DELAY_US = 2000;

// --- Pulse detection ---
// long = signed 32-bit integer (large IR sensor readings)
constexpr long FINGER_IR_MIN = 70000;
constexpr int PULSE_BPM_MIN = 60;

// enum class = typed list of named constants (safer than plain #define)
// : uint8_t stores each value in one byte
enum class ButtonId : uint8_t { Start, Gemuk, Stop, Kurus };

// Momentary push-button events from pollButtons()
enum class ButtonEvent : uint8_t { None, Start, Stop };

// Main CPR state machine states
enum class SystemState : uint8_t {
  Idle,
  PulseCheck,
  AwaitingMode,
  BeltTighten,
  RunningGemuk,
  RunningKurus,
};
