#pragma once

#include <Arduino.h>

bool pulseSensorInit();
void pulseSensorUpdate();
void pulseSensorReset();
bool hasDetectablePulse();

long pulseSensorIrValue();
int pulseSensorBeatAvg();
float pulseSensorBpm();
