#include "display.h"

#include <LiquidCrystal_I2C.h>

#include "config.h"
#include "pulse_sensor.h"

// Object instance: ClassName objectName(args) — creates LCD driver at I2C addr 0x27, 16 cols, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

void printBpmLine2() {
  const long irValue = pulseSensorIrValue();
  const int beatAvg = pulseSensorBeatAvg();
  const float beatsPerMinute = pulseSensorBpm();  // float = decimal number

  lcd.setCursor(0, 1);  // column 0, row 1 (second line)
  lcd.print(F("BPM: "));
  if (irValue < FINGER_IR_MIN) {
    lcd.print(F("--"));  // no finger on sensor
  } else {
    // Prefer rolling average; fallback to instant BPM if avg not ready yet
    lcd.print(beatAvg > 0 ? beatAvg : (int)beatsPerMinute);  // (int) = C-style cast to integer
  }
  lcd.print(F("        "));  // trailing spaces erase old digits
}

void displayInit() {
  lcd.init();
  lcd.backlight();
  showIdleDisplay();
}

void showIdleDisplay() {
  lcd.setCursor(0, 0);
  lcd.print(F("ALAT CPR OTMTS        "));
  printBpmLine2();
}

void showAliveDisplay() {
  lcd.setCursor(0, 0);
  lcd.print(F("Pasien Masih Hid"));
  printBpmLine2();
}

void showArrestDisplay() {
  lcd.setCursor(0, 0);
  lcd.print(F("Pasien Henti Jnt"));
  lcd.setCursor(0, 1);
  lcd.print(F("Gemuk / Kurus   "));
}

// __FlashStringHelper* = pointer to flash-stored string (from F("...") in caller)
void showCprDisplay(const __FlashStringHelper *modeLabel, uint8_t current, uint8_t total, uint8_t cycle) {
  lcd.setCursor(0, 0);
  lcd.print(modeLabel);
  lcd.setCursor(0, 1);
  lcd.print(F("Komp:"));
  lcd.print(current);
  lcd.print(F("/"));
  lcd.print(total);
  lcd.print(F(" C:"));
  lcd.print(cycle);
  lcd.print(F("   "));
}

void showStoppedDisplay() {
  lcd.setCursor(0, 0);
  lcd.print(F("CPR dihentikan  "));
  lcd.setCursor(0, 1);
  lcd.print(F("                "));
}

void showPulseCheckDisplay() {
  lcd.setCursor(0, 0);
  lcd.print(F("Cek denyut...   "));
  printBpmLine2();
}

void showSensorNotFoundDisplay() {
  lcd.setCursor(0, 0);
  lcd.print(F("Sensor not found"));
  lcd.setCursor(0, 1);
  lcd.print(F("                "));
}

void showPulsePauseDisplay() {
  lcd.setCursor(0, 0);
  lcd.print(F("Denyut terdeteksi"));
  printBpmLine2();
}

void showBeltTightenDisplay(bool gemuk, float weightKg) {
  lcd.setCursor(0, 0);
  lcd.print(F("Kencang sabuk   "));
  lcd.setCursor(0, 1);
  lcd.print(F("Berat "));
  lcd.print(weightKg, 1);
  lcd.print(F("/"));
  lcd.print(gemuk ? GEMUK_MIN_WEIGHT_KG : KURUS_MIN_WEIGHT_KG, 1);
  lcd.print(F("kg  "));
}
