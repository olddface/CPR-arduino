# Automatic CPR Portable

Firmware for **Automatic CPR Portable Dilengkapi dengan Sensor Denyut Jantung** — Arduino Uno prototype with MAX30102 heart-rate monitoring, I2C LCD, HX711 load cell belt-tension gate, TB6600 stepper belt compression, push buttons, and active buzzer.

## Project structure

```
test-max30102/
├── platformio.ini          # Board, framework, library deps
├── src/
│   ├── main.cpp            # Setup / loop entry point
│   ├── cpr_engine.cpp      # State machine (idle → pulse check → CPR)
│   ├── pulse_sensor.cpp    # MAX30102 init and BPM averaging
│   ├── stepper_motor.cpp   # TB6600 timed step pulses
│   ├── load_cell.cpp       # HX711 belt-tension read / threshold gate
│   ├── display.cpp         # I2C LCD messages
│   ├── buttons.cpp         # Debounced button polling
│   └── buzzer.cpp          # Ventilation / stop beeps
└── include/
    ├── config.h            # Pins, timing, CPR constants (edit here)
    ├── cpr_engine.h
    ├── pulse_sensor.h
    ├── stepper_motor.h
    ├── load_cell.h
    ├── display.h
    ├── buttons.h
    └── buzzer.h
```

| Module | Responsibility |
|--------|----------------|
| `cpr_engine` | System state machine and compression batch orchestration |
| `pulse_sensor` | MAX30102 setup, beat detection, rolling BPM average |
| `stepper_motor` | TB6600 enable/step/dir control; ENA on only during compression strokes; stops if belt tension lost |
| `load_cell` | HX711 init, tare, weight read, and minimum-tension gate for Gemuk/Kurus |
| `display` | LCD screens for each state |
| `buttons` | Momentary Start/Stop + toggle Gemuk/Kurus reads |
| `buzzer` | Active buzzer on/off and multi-beep patterns |

## Hardware

| Component | Connection |
|-----------|------------|
| Arduino Uno | Main controller |
| MAX30102 | 5V, GND, SDA → A4, SCL → A5 |
| LCD 16×2 I2C | Same I2C bus, address `0x27` (try `0x3F` if blank) |
| Active buzzer | D13 → buzzer+, GND → buzzer− |
| TB6600 PUL | PUL+ → D9 (`STEP_PIN`), PUL− → GND (with DIR−) |
| TB6600 DIR | DIR+ → D8 (`DIR_PIN`), DIR− → GND (with PUL−) |
| TB6600 ENA | ENA+ → D10 (`ENABLE_PIN`), ENA− → GND |
| TB6600 common | PUL−, DIR−, and ENA− to Arduino GND; driver GND tied to Arduino GND |
| Stepper NEMA 23 | A+/A−, B+/B− to driver |
| Motor PSU | 9–42 V DC to driver VCC/GND (**not** Arduino 5V) |
| HX711 + 50 kg load cell | VCC → 5V, GND → GND, DT → D5 (`HX711_DT_PIN`), SCK → D6 (`HX711_SCK_PIN`); E+/E− and A+/A− to load cell per module silkscreen |

### HX711 load cell (belt tension gate)

After **Gemuk** or **Kurus** is selected, the firmware enters a **belt tighten** phase. The **stepper motor runs automatically** to tighten the belt while the load cell is read. When weight reaches the minimum for that mode, the motor **stops** and CPR compressions begin.

During CPR, the stepper only runs while belt tension stays above the threshold. If tension drops mid-stroke, the motor stops immediately.

| Mode | Default minimum weight | Constant |
|------|------------------------|----------|
| Gemuk | ~1.0 kg | `GEMUK_MIN_WEIGHT_KG` |
| Kurus | ~0.5 kg | `KURUS_MIN_WEIGHT_KG` |

**Calibration:** `LOAD_CELL_SCALE` in `config.h` is a placeholder. With a known mass on the cell (e.g. 1 kg), read the raw value from serial, then set:

```
LOAD_CELL_SCALE = (raw_reading - offset) / known_kg
```

Tare runs automatically when entering belt tighten (after mode select). Re-tune `LOAD_CELL_SCALE` until serial `W=` matches your known mass.

### TB6600 enable (ENA)

The driver is **disabled at boot and idle** so the motor coils are not energized between compressions. `stepMotorTimed()` turns ENA **on** at the start of each stroke and **off** when the stroke finishes (or when **Stop** is pressed mid-stroke).

| Situation | Expected |
|-----------|----------|
| Idle (Arduino running, motor PSU on) | Shaft spins easily by hand; little or no chopper hum |
| During compression stroke | Shaft resists; driver hums while stepping |
| Motor PSU only (Arduino off) | Driver may stay enabled — **not** a valid ENA test |

If the shaft stays stiff at idle, toggle `TB6600_ENABLE_5V` in `config.h`. If still stiff, check ENA+ on D10, ENA− on GND, and common GND to the driver.

If your board uses PUL+/DIR+ to 5V instead (PUL−/DIR− on GPIO), set `TB6600_COMMON_5V` to `true` and re-check ENA polarity.

### Buttons (INPUT_PULLUP — contact to GND = ON)

| Pin | Type | Function |
|-----|------|----------|
| D2 | **Momentary** | **Start** — press once to begin pulse check |
| D7 | **Momentary** | **Stop** — press once to abort (also checked while motor runs) |
| D4 | **Toggle** | **Gemuk** — flip ON to run 30-compression mode |
| D12 | **Toggle** | **Kurus** — flip ON to run 15-compression mode |

Start and Stop are push buttons: firmware reacts on **press**, not while held (except Stop aborts mid-stroke via `isStopPressed()` inside the motor loop). Gemuk/Kurus are toggle switches: firmware reads whether the switch is **ON**.

## System flow

```
Power on → Idle (show BPM)
    ↓ [Start]
Pulse check (2.5 s)
    ├─ Pulse detected → "Pasien Masih Hidup" → Idle
    └─ No pulse       → "Pasien Henti Jantung" → pick Gemuk or Kurus
                              ↓
                    Belt tighten (motor runs → stops at threshold)
                    ├─ Motor steps until load cell ≥ minimum
                    ├─ LCD: "Kencang sabuk" + live weight vs minimum
                    ├─ Gemuk: stop at ~1.0 kg
                    └─ Kurus: stop at ~0.5 kg
                              ↓ [threshold met, motor stops]
                    Compression loop (until Stop or tension lost)
                    ├─ Gemuk: 30 kompresi
                    └─ Kurus: 15 kompresi
                              ↓
                    Buzzer 2× (ventilasi cue)
                              ↓
                    Repeat or Stop → Idle
```

### LCD messages

| State | Line 1 | Line 2 |
|-------|--------|--------|
| Idle | `ALAT CPR OTOMAT` | `BPM: xx` |
| Checking | `Cek denyut...` | `BPM: xx` |
| Alive | `Pasien Masih Hid` | `BPM: xx` |
| Arrest | `Pasien Henti Jnt` | `Gemuk / Kurus` |
| Belt tighten | `Kencang sabuk` | `Berat x.x/x.xkg` (live / minimum) |
| Gemuk CPR | `Mode Gemuk` | `Komp: n/30` |
| Kurus CPR | `Mode Kurus` | `Komp: n/15` |
| Stopped | `CPR dihentikan` | (blank) |

## Tunable parameters

Edit constants in [`include/config.h`](include/config.h):

| Constant | Default | Purpose |
|----------|---------|---------|
| `GEMUK_COMPRESSIONS` | 30 | Compressions per cycle (gemuk / larger torso) |
| `KURUS_COMPRESSIONS` | 15 | Compressions per cycle (kurus / smaller torso) |
| `COMPRESSIONS_PER_MIN` | 110 | Target rate (AHA: 100–120/min) |
| `STEPS_PER_STROKE` | 200 | Motor steps per belt direction — tune to belt travel |
| `STEPS_PER_REV` | 200 | Must match TB6600 DIP switch microstep setting |
| `FINGER_IR_MIN` | 70000 | IR threshold for finger / pulse detection |
| `PULSE_BPM_MIN` | 20 | Minimum BPM to count as detectable pulse |
| `ENABLE_PIN` | `10` | TB6600 ENA+ (Arduino GPIO) |
| `TB6600_COMMON_5V` | `false` | Set `true` if PUL+/DIR+ go to 5V and PUL−/DIR− go to GPIO |
| `TB6600_ENABLE_5V` | `false` | ENA opto polarity — toggle if idle shaft stays stiff (driver still on) |
| `HX711_DT_PIN` | `5` | HX711 data pin |
| `HX711_SCK_PIN` | `6` | HX711 clock pin |
| `LOAD_CELL_SCALE` | `-7050.0` | Calibration factor — tune with known mass |
| `GEMUK_MIN_WEIGHT_KG` | `1.0` | Minimum belt tension (kg) — motor stops tightening at this value |
| `KURUS_MIN_WEIGHT_KG` | `0.5` | Minimum belt tension (kg) — motor stops tightening at this value |
| `BELT_TIGHTEN_STEP_DELAY_US` | `2000` | Step interval while auto-tightening belt (µs) |

## Build and upload

Requires [PlatformIO](https://platformio.org/).

```bash
cd test-max30102
pio run              # build
pio run -t upload    # flash to Arduino
pio device monitor   # serial log @ 115200 baud
```

Or from VS Code / Cursor: **PlatformIO → Build**, then **Upload**.

## Serial monitor

At 115200 baud the firmware prints state, IR value, BPM, and load-cell weight each UI tick:

```
state=0 IR=125000 BPM=72 W=0.00
```

State values: `0` Idle, `1` PulseCheck, `2` AwaitingMode, `3` BeltTighten, `4` RunningGemuk, `5` RunningKurus.

## Test procedure

1. **Idle** — LCD shows title and BPM when finger is on MAX30102.
2. **Alive path** — Finger on sensor → press **Start** → `Pasien Masih Hidup`, motor does not run.
3. **Arrest path** — No finger → **Start** → `Pasien Henti Jantung` → flip **Gemuk** or **Kurus** toggle ON.
4. **Belt tighten** — Motor runs to tighten the belt; LCD shows live weight. Motor stops when threshold is reached (Gemuk ~1 kg, Kurus ~0.5 kg), then CPR starts.
5. **Compression** — Motor runs tighten/release strokes only while belt tension stays above minimum; LCD counts `Komp: n/total`.
6. **Ventilation cue** — After each batch, buzzer beeps twice.
7. **Stop / tension lost** — Press **Stop** anytime, or loosen belt below threshold → motor halts, short buzzer chirp, returns to idle after 2 s.

## Dependencies

From [`platformio.ini`](platformio.ini):

- `sparkfun/SparkFun MAX3010x Pulse and Proximity Sensor Library`
- `marcoschwartz/LiquidCrystal_I2C`
- `bogde/HX711`

## Notes

- CPR is **button-driven**, not auto-triggered by low BPM.
- Stepper **auto-tightens** the belt after mode select; stops when load cell hits threshold.
- During CPR, stepper compressions require **continuous belt tension** on the load cell.
- Buzzer cues ventilation only; there is no bag-valve hardware.
- Belt depth (5–6 cm target) is set mechanically via `STEPS_PER_STROKE` and mechanical linkage, not in software.
- If motor does not move, verify TB6600 opto wiring (GND common vs 5V common) and set `TB6600_COMMON_5V` accordingly.
- Connect ENA+ to D10 and ENA− to GND for software enable control. Leaving ENA unconnected keeps the driver always enabled.
- Motor PSU alone can produce chopper hum even when idle; verify ENA with **both** PSUs on and Arduino at idle (shaft should turn freely).

`.pio/` build cache and local VS Code generated files are gitignored. Run `pio run` after clone to restore dependencies.
