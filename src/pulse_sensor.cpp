#include "pulse_sensor.h"

#include <Wire.h>

#include "config.h"
#include "heartRate.h"
#include "MAX30105.h"

MAX30105 particleSensor;

constexpr byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;

float beatsPerMinute;
int beatAvg;
long irValue = 0;

bool pulseSensorInit() {
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    return false;
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);
  return true;
}

void pulseSensorUpdate() {
  irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    const long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > PULSE_BPM_MIN) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[x];
      }
      beatAvg /= RATE_SIZE;
    }
  }
}

void pulseSensorReset() {
  rateSpot = 0;
  beatAvg = 0;
  beatsPerMinute = 0;
  for (byte x = 0; x < RATE_SIZE; x++) {
    rates[x] = 0;
  }
}

bool hasDetectablePulse() {
  return irValue >= FINGER_IR_MIN && beatAvg >= PULSE_BPM_MIN;
}

long pulseSensorIrValue() {
  return irValue;
}

int pulseSensorBeatAvg() {
  return beatAvg;
}

float pulseSensorBpm() {
  return beatsPerMinute;
}
