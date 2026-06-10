#pragma once

#include <Arduino.h>

void displayInit();
void showIdleDisplay();
void showAliveDisplay();
void showArrestDisplay();
void showCprDisplay(const __FlashStringHelper *modeLabel, uint8_t current, uint8_t total);
void showStoppedDisplay();
void showPulseCheckDisplay();
void showSensorNotFoundDisplay();
