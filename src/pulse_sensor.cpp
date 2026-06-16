#include "pulse_sensor.h"

#include <Wire.h>

#include "config.h"
#include "heartRate.h"
#include "MAX30105.h"

// Global sensor object from SparkFun library
MAX30105 particleSensor;

constexpr byte RATE_SIZE = 4;  // byte = Arduino alias for uint8_t
byte rates[RATE_SIZE];         // circular buffer of last BPM readings
byte rateSpot = 0;             // next slot to write in rates[]
long lastBeat = 0;

float beatsPerMinute;  // latest single-beat estimate
int beatAvg;           // rolling average BPM
long irValue = 0;      // raw infrared level from sensor

bool pulseSensorInit() {
  // begin returns false if I2C device not found at expected address
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    return false;
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);  // 0x = hexadecimal literal
  particleSensor.setPulseAmplitudeGreen(0);
  return true;  // success
}

void pulseSensorUpdate() {
  irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    const long delta = millis() - lastBeat;  // ms since previous beat
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);  // 1000.0 forces float division

    if (beatsPerMinute < 255 && beatsPerMinute > PULSE_BPM_MIN) {
      rates[rateSpot++] = (byte)beatsPerMinute;  // post-increment: use then add 1
      rateSpot %= RATE_SIZE;  // %= wrap index: 0,1,2,3,0,1,...

      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[x];  // sum all slots (including zeros at start)
      }
      beatAvg /= RATE_SIZE;  // integer division → average BPM
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
  // && = both conditions must be true
  return irValue >= FINGER_IR_MIN && beatAvg >= PULSE_BPM_MIN;
}

bool isPulseBelowCprThreshold() {
  return irValue >= FINGER_IR_MIN && beatAvg < PULSE_BPM_MIN;
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
