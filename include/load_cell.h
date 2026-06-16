#pragma once

#include <Arduino.h>

void loadCellInit();
void loadCellTare();
void loadCellSetGemukMode(bool gemuk);
float loadCellWeightKg();
bool loadCellMeetsThreshold();
bool loadCellAllowsMotor();
