#pragma once

#include <Arduino.h>

// stepper_motor.cpp — TB6600 pulse, direction, and enable control
void stepperInit();
unsigned long compressionStepDelayUs();
bool stepMotorTimed(uint16_t steps, bool directionUp, unsigned long delayUs);
