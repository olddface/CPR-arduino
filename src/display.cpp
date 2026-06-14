#include "display.h"

#include <LiquidCrystal_I2C.h>

#include "config.h"
#include "pulse_sensor.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

void printBpmLine2() {
  const long irValue = pulseSensorIrValue();
  const int beatAvg = pulseSensorBeatAvg();
  const float beatsPerMinute = pulseSensorBpm();

  lcd.setCursor(0, 1);
  lcd.print(F("BPM: "));
  if (irValue < FINGER_IR_MIN) {
    lcd.print(F("--"));
  } else {
    lcd.print(beatAvg > 0 ? beatAvg : (int)beatsPerMinute);
  }
  lcd.print(F("        "));
}

void displayInit() {
  lcd.init();
  lcd.backlight();
  showIdleDisplay();
}

void showIdleDisplay() {
  lcd.setCursor(0, 0);
  lcd.print(F("ALAT CPR OTMTS"));
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

void showCprDisplay(const __FlashStringHelper *modeLabel, uint8_t current, uint8_t total) {
  lcd.setCursor(0, 0);
  lcd.print(modeLabel);
  lcd.setCursor(0, 1);
  lcd.print(F("Komp: "));
  lcd.print(current);
  lcd.print(F("/"));
  lcd.print(total);
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
