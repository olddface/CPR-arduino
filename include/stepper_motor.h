#pragma once

#include <Arduino.h>

void stepperInit();
unsigned long compressionStepDelayUs();
bool stepMotorTimed(uint16_t steps, bool directionUp, unsigned long delayUs);
