#include "cpr_engine.h"

#include <Arduino.h>

#include "buttons.h"
#include "buzzer.h"
#include "config.h"
#include "display.h"
#include "pulse_sensor.h"
#include "stepper_motor.h"

SystemState systemState = SystemState::Idle;
uint32_t pulseCheckStartMs = 0;
uint32_t stoppedDisplayUntilMs = 0;
uint32_t lastUiUpdateMs = 0;

void handleStop() {
  buzzerBeep(1, 400, 0);
  showStoppedDisplay();
  stoppedDisplayUntilMs = millis() + STOP_DISPLAY_MS;
  Serial.println(F("CPR stopped"));
}

bool runCompressionBatch(uint8_t count, uint8_t total) {
  const unsigned long stepDelayUs = compressionStepDelayUs();

  for (uint8_t i = 0; i < count; i++) {
    showCprDisplay(
        systemState == SystemState::RunningGemuk ? F("Mode Gemuk     ") : F("Mode Kurus     "),
        i + 1, total);

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
  return systemState;
}

void cprEngineUpdate() {
  pulseSensorUpdate();
  const ButtonEvent btn = pollButtons();
  const uint32_t now = millis();

  switch (systemState) {
    case SystemState::Idle:
      if (now < stoppedDisplayUntilMs) {
        break;
      }
      if (now - lastUiUpdateMs >= UI_UPDATE_MS) {
        lastUiUpdateMs = now;
        showIdleDisplay();
      }
      if (btn == ButtonEvent::Start) {
        pulseSensorReset();
        pulseCheckStartMs = now;
        systemState = SystemState::PulseCheck;
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
      if (btn == ButtonEvent::Gemuk) {
        systemState = SystemState::RunningGemuk;
        Serial.println(F("Gemuk CPR mode"));
      } else if (btn == ButtonEvent::Kurus) {
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

      if (btn == ButtonEvent::Stop || isStopPressed()) {
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

      if (btn == ButtonEvent::Stop || isStopPressed()) {
        handleStop();
        systemState = SystemState::Idle;
      }
      break;
  }

  if (now - lastUiUpdateMs >= UI_UPDATE_MS) {
    lastUiUpdateMs = now;

    Serial.print(F("state="));
    Serial.print(static_cast<uint8_t>(systemState));
    Serial.print(F(" IR="));
    Serial.print(pulseSensorIrValue());
    Serial.print(F(" BPM="));
    Serial.print(pulseSensorBeatAvg());
    Serial.println();
  }
}
