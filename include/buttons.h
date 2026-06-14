#pragma once

#include "config.h"

void buttonsInit();
ButtonEvent pollButtons();

// Momentary push buttons
bool isStopPressed();

// Toggle switches (LOW = switch ON)
bool isGemukSelected();
bool isKurusSelected();
