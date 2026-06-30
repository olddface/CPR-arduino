#include "cpr_engine.h"

#include <Arduino.h>

#include "buttons.h"
#include "buzzer.h"
#include "config.h"
#include "display.h"
#include "pulse_sensor.h"
#include "load_cell.h"
#include "stepper_motor.h"

// Global variables — live for whole program; shared across functions in this file
SystemState systemState = SystemState::Idle;
uint32_t pulseCheckStartMs = 0;
uint32_t stoppedDisplayUntilMs = 0;
uint32_t lastUiUpdateMs = 0;
uint32_t pulsePauseStartMs = 0;
bool beltTightenGemuk = false;
SystemState pausedFromState = SystemState::RunningGemuk;
uint8_t cprCycleCount = 1;

enum class BatchResult : uint8_t { Complete, StoppedByUser, StoppedByPulse };

void handleStop();

void enterPulsePause(SystemState fromState, uint32_t now) {
  pausedFromState = fromState;
  pulsePauseStartMs = now;
  pulseSensorReset();
  buzzerBeep(1, 400, 0);
  showPulsePauseDisplay();
  Serial.print(F("Pulse detected (BPM>="));
  Serial.print(PULSE_BPM_MIN);
  Serial.println(F(") — CPR paused"));
  systemState = SystemState::PulsePause;
}

void handleRunningComplete() {
  const ButtonEvent btn = pollButtons();
  if (btn == ButtonEvent::Stop) {
    handleStop();
    systemState = SystemState::Idle;
  }
}

void handleStop() {
  buzzerBeep(1, 400, 0);  // one short beep: 400 ms on, 0 ms off between
  showStoppedDisplay();
  stoppedDisplayUntilMs = millis() + STOP_DISPLAY_MS;  // show "stopped" for 2 s
  Serial.println(F("CPR stopped"));
}

// bool return: true = batch finished, false = user hit Stop mid-batch
BatchResult runCompressionBatch(uint8_t count, uint8_t total) {
  const unsigned long stepDelayUs = compressionStepDelayUs();

  for (uint8_t i = 0; i < count; i++) {
    // Nested ternary picks LCD title based on current enum state
    showCprDisplay(
        systemState == SystemState::RunningGemuk ? F("Mode Gemuk     ") : F("Mode Kurus     "),
        i + 1, total, cprCycleCount);  // i+1 because humans count from 1, loop i starts at 0

    if (!stepMotorTimed(STEPS_PER_STROKE, true, stepDelayUs)) {
      return hasDetectablePulse() ? BatchResult::StoppedByPulse : BatchResult::StoppedByUser;
    }
    if (!stepMotorTimed(STEPS_PER_STROKE, false, stepDelayUs)) {
      return hasDetectablePulse() ? BatchResult::StoppedByPulse : BatchResult::StoppedByUser;
    }

    pulseSensorUpdate();
    if (hasDetectablePulse()) {
      return BatchResult::StoppedByPulse;
    }
  }

  return BatchResult::Complete;
}

void cprEngineInit() {
  systemState = SystemState::Idle;
  pulseCheckStartMs = 0;
  stoppedDisplayUntilMs = 0;
  lastUiUpdateMs = 0;
  pulsePauseStartMs = 0;
  pausedFromState = SystemState::RunningGemuk;
  cprCycleCount = 1;
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
        beltTightenGemuk = true;
        loadCellSetGemukMode(true);
        loadCellTare();
        systemState = SystemState::BeltTighten;
        Serial.println(F("Gemuk — tighten belt (~1 kg)"));
      } else if (isKurusSelected()) {
        beltTightenGemuk = false;
        loadCellSetGemukMode(false);
        loadCellTare();
        systemState = SystemState::BeltTighten;
        Serial.println(F("Kurus — tighten belt"));
      } else if (btn == ButtonEvent::Stop) {
        handleStop();
        systemState = SystemState::Idle;
      }
      break;

    case SystemState::BeltTighten:
      if (btn == ButtonEvent::Stop) {
        handleStop();
        systemState = SystemState::Idle;
        break;
      }

      showBeltTightenDisplay(beltTightenGemuk, loadCellWeightKg());

      if (stepMotorUntilWeight(true, BELT_TIGHTEN_STEP_DELAY_US, beltTightenGemuk)) {
        systemState = beltTightenGemuk ? SystemState::RunningGemuk : SystemState::RunningKurus;
        Serial.print(F("Belt tension OK — "));
        Serial.print(loadCellWeightKg(), 2);
        Serial.println(F(" kg — starting CPR"));
      } else {
        handleStop();
        systemState = SystemState::Idle;
      }
      break;

    case SystemState::RunningGemuk: {
      const BatchResult batch = runCompressionBatch(GEMUK_COMPRESSIONS, GEMUK_COMPRESSIONS);
      if (batch == BatchResult::StoppedByPulse) {
        enterPulsePause(SystemState::RunningGemuk, now);
        break;
      }
      if (batch == BatchResult::StoppedByUser) {
        handleStop();
        systemState = SystemState::Idle;
        break;
      }

      buzzerBeep(VENTILATION_BEEPS);
      cprCycleCount++;
      handleRunningComplete();
      break;
    }

    case SystemState::RunningKurus: {
      const BatchResult batch = runCompressionBatch(KURUS_COMPRESSIONS, KURUS_COMPRESSIONS);
      if (batch == BatchResult::StoppedByPulse) {
        enterPulsePause(SystemState::RunningKurus, now);
        break;
      }
      if (batch == BatchResult::StoppedByUser) {
        handleStop();
        systemState = SystemState::Idle;
        break;
      }

      buzzerBeep(VENTILATION_BEEPS);
      cprCycleCount++;
      handleRunningComplete();
      break;
    }

    case SystemState::PulsePause:
      if (btn == ButtonEvent::Stop) {
        handleStop();
        systemState = SystemState::Idle;
        break;
      }

      if (now - lastUiUpdateMs >= UI_UPDATE_MS) {
        lastUiUpdateMs = now;
        showPulsePauseDisplay();
      }

      if (now - pulsePauseStartMs >= PULSE_PAUSE_WAIT_MS && isPulseBelowCprThreshold()) {
        Serial.println(F("Pulse below threshold — resuming CPR from komp 0"));
        systemState = pausedFromState;
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
    Serial.print(F(" W="));
    Serial.print(loadCellWeightKg(), 2);
    Serial.println();  // newline after debug row
  }
}
