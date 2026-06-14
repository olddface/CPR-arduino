#include "cpr_engine.h"

#include <Arduino.h>

#include "buttons.h"
#include "buzzer.h"
#include "config.h"
#include "display.h"
#include "pulse_sensor.h"
#include "stepper_motor.h"

// Global variables — live for whole program; shared across functions in this file
SystemState systemState = SystemState::Idle;
uint32_t pulseCheckStartMs = 0;
uint32_t stoppedDisplayUntilMs = 0;
uint32_t lastUiUpdateMs = 0;

void handleStop() {
  buzzerBeep(1, 400, 0);  // one short beep: 400 ms on, 0 ms off between
  showStoppedDisplay();
  stoppedDisplayUntilMs = millis() + STOP_DISPLAY_MS;  // show "stopped" for 2 s
  Serial.println(F("CPR stopped"));
}

// bool return: true = batch finished, false = user hit Stop mid-batch
bool runCompressionBatch(uint8_t count, uint8_t total) {
  const unsigned long stepDelayUs = compressionStepDelayUs();

  for (uint8_t i = 0; i < count; i++) {
    // Nested ternary picks LCD title based on current enum state
    showCprDisplay(
        systemState == SystemState::RunningGemuk ? F("Mode Gemuk     ") : F("Mode Kurus     "),
        i + 1, total);  // i+1 because humans count from 1, loop i starts at 0

    // !stepMotorTimed(...) = "if stepMotorTimed returned false"
    if (!stepMotorTimed(STEPS_PER_STROKE, true, stepDelayUs)) {
      return false;
    }
    if (!stepMotorTimed(STEPS_PER_STROKE, false, stepDelayUs)) {
      return false;
    }

    pulseSensorUpdate();
  }

  return true;
}

void cprEngineInit() {
  systemState = SystemState::Idle;
  pulseCheckStartMs = 0;
  stoppedDisplayUntilMs = 0;
  lastUiUpdateMs = 0;
}

SystemState cprEngineState() {
  return systemState;  // getter: other files could read state (optional use)
}

void cprEngineUpdate() {
  pulseSensorUpdate();
  const ButtonEvent btn = pollButtons();
  const uint32_t now = millis();

  // switch on enum — like multiple if/else on same variable
  switch (systemState) {
    case SystemState::Idle:
      if (now < stoppedDisplayUntilMs) {
        break;  // still showing "CPR stopped" message — skip idle screen
      }
      if (now - lastUiUpdateMs >= UI_UPDATE_MS) {
        lastUiUpdateMs = now;
        showIdleDisplay();
      }
      if (btn == ButtonEvent::Start) {
        pulseSensorReset();
        pulseCheckStartMs = now;
        systemState = SystemState::PulseCheck;  // change state
        Serial.println(F("Start — checking pulse"));
      }
      break;

    case SystemState::PulseCheck:
      if (now - lastUiUpdateMs >= UI_UPDATE_MS) {
        lastUiUpdateMs = now;
        showPulseCheckDisplay();
      }

      if (btn == ButtonEvent::Stop) {
        handleStop();
        systemState = SystemState::Idle;
        break;
      }

      if (now - pulseCheckStartMs >= PULSE_CHECK_MS) {
        if (hasDetectablePulse()) {
          showAliveDisplay();
          Serial.print(F("Pulse detected — BPM="));
          Serial.println(pulseSensorBeatAvg());
          systemState = SystemState::Idle;
        } else {
          showArrestDisplay();
          Serial.println(F("No pulse — awaiting mode"));
          systemState = SystemState::AwaitingMode;
        }
      }
      break;

    case SystemState::AwaitingMode:
      if (isGemukSelected()) {
        systemState = SystemState::RunningGemuk;
        Serial.println(F("Gemuk CPR mode"));
      } else if (isKurusSelected()) {
        systemState = SystemState::RunningKurus;
        Serial.println(F("Kurus CPR mode"));
      } else if (btn == ButtonEvent::Stop) {
        handleStop();
        systemState = SystemState::Idle;
      }
      break;

    case SystemState::RunningGemuk:
      if (!runCompressionBatch(GEMUK_COMPRESSIONS, GEMUK_COMPRESSIONS)) {
        handleStop();
        systemState = SystemState::Idle;
        break;
      }

      buzzerBeep(VENTILATION_BEEPS);

      if (btn == ButtonEvent::Stop) {
        handleStop();
        systemState = SystemState::Idle;
      }
      break;

    case SystemState::RunningKurus:
      if (!runCompressionBatch(KURUS_COMPRESSIONS, KURUS_COMPRESSIONS)) {
        handleStop();
        systemState = SystemState::Idle;
        break;
      }

      buzzerBeep(VENTILATION_BEEPS);

      if (btn == ButtonEvent::Stop) {
        handleStop();
        systemState = SystemState::Idle;
      }
      break;
  }

  // Debug line every UI_UPDATE_MS (runs in all states)
  if (now - lastUiUpdateMs >= UI_UPDATE_MS) {
    lastUiUpdateMs = now;

    Serial.print(F("state="));
    // Cast enum to number for printing (0=Idle, 1=PulseCheck, ...)
    Serial.print(static_cast<uint8_t>(systemState));
    Serial.print(F(" IR="));
    Serial.print(pulseSensorIrValue());
    Serial.print(F(" BPM="));
    Serial.print(pulseSensorBeatAvg());
    Serial.println();  // newline after debug row
  }
}
