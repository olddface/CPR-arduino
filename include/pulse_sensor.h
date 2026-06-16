#pragma once

#include <Arduino.h>

// pulse_sensor.cpp — MAX30102 IR and BPM averaging
bool pulseSensorInit();
void pulseSensorUpdate();
void pulseSensorReset();
bool hasDetectablePulse();
bool isPulseBelowCprThreshold();

long pulseSensorIrValue();
int pulseSensorBeatAvg();
float pulseSensorBpm();
