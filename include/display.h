#pragma once

#include <Arduino.h>

// display.cpp — I2C LCD text for each CPR screen
void displayInit();
void showIdleDisplay();
void showAliveDisplay();
void showArrestDisplay();
void showCprDisplay(const __FlashStringHelper *modeLabel, uint8_t current, uint8_t total, uint8_t cycle);
void showStoppedDisplay();
void showPulseCheckDisplay();
void showSensorNotFoundDisplay();
void showBeltTightenDisplay(bool gemuk, float weightKg);
void showPulsePauseDisplay();
