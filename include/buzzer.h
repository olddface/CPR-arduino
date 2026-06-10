#pragma once

#include "config.h"

void buzzerInit();
void buzzerOn();
void buzzerOff();
void buzzerBeep(uint8_t count, uint32_t onMs = BEEP_ON_MS, uint32_t offMs = BEEP_OFF_MS);
