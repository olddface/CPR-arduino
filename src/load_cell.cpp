#include "load_cell.h"

#include <HX711.h>

#include "config.h"

namespace {

HX711 scale;
float minWeightKg = GEMUK_MIN_WEIGHT_KG;
float lastWeightKg = 0.0f;

}  // namespace

void loadCellInit() {
  scale.begin(HX711_DT_PIN, HX711_SCK_PIN);
  scale.set_scale(LOAD_CELL_SCALE);
  scale.tare();
  lastWeightKg = 0.0f;
}

void loadCellTare() {
  scale.tare();
  lastWeightKg = 0.0f;
}

void loadCellSetGemukMode(bool gemuk) {
  minWeightKg = gemuk ? GEMUK_MIN_WEIGHT_KG : KURUS_MIN_WEIGHT_KG;
}

float loadCellWeightKg() {
  if (scale.is_ready()) {
    lastWeightKg = scale.get_units(1);
  }
  return lastWeightKg;
}

bool loadCellMeetsThreshold() {
  return loadCellWeightKg() >= minWeightKg;
}

bool loadCellAllowsMotor() {
  return loadCellMeetsThreshold();
}
