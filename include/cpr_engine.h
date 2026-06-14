#pragma once

#include "config.h"

// Function declarations only (no body) — tells compiler these exist in cpr_engine.cpp
void cprEngineInit();
void cprEngineUpdate();

SystemState cprEngineState();
